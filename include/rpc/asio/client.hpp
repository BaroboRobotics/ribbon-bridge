#ifndef RPC_ASIO_CLIENT_HPP
#define RPC_ASIO_CLIENT_HPP

#include <util/asio/asynccompletion.hpp>
#include <util/asio/operation.hpp>
#include <util/asio/transparentservice.hpp>
#include <util/producerconsumerqueue.hpp>

#include "rpc/componenttraits.hpp"
#include "rpc/message.hpp"
#include "rpc/system_error.hpp"
#include "rpc/version.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <boost/optional.hpp>

#include <atomic>
#include <map>
#include <memory>
#include <queue>
#include <utility>

#include <boost/asio/yield.hpp>

namespace rpc { namespace asio {

using namespace std::placeholders;

template <class MessageQueue>
struct ClientImpl : public std::enable_shared_from_this<ClientImpl<MessageQueue>> {
    using RequestId = uint32_t;

    explicit ClientImpl (boost::asio::io_service& context)
        : mMessageQueue(context)
    {
        mLog.add_attribute("Protocol", boost::log::attributes::constant<std::string>("RB-CL"));
    }

    void close (boost::system::error_code& ec) {
        mMessageQueue.close(ec);
        mMessageQueue.get_io_service().post([self=this->shared_from_this(), this] {
            voidHandlers(boost::asio::error::operation_aborted);
        });
    }

    MessageQueue& messageQueue () {
        return mMessageQueue;
    }
    const MessageQueue& messageQueue () const {
        return mMessageQueue;
    }

    boost::log::sources::logger& log () {
        return mLog;
    }

    struct SendRequestOperation;

    template <class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::system::error_code))
    asyncSendRequest (RequestId requestId, barobo_rpc_Request request, CompletionToken&& token) {
        util::asio::AsyncCompletion<
            CompletionToken, void(boost::system::error_code)
        > init { std::forward<CompletionToken>(token) };

        using Op = SendRequestOperation;
        util::asio::makeOperation<Op>(std::move(init.handler),
            this->shared_from_this(), requestId, request)();

        return init.result.get();
    }

    template <class Duration, class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
        void(boost::system::error_code, boost::optional<barobo_rpc_Reply>))
    asyncReceiveReply (RequestId requestId, Duration&& timeout, CompletionToken&& token) {
        util::asio::AsyncCompletion<
            CompletionToken, void(boost::system::error_code, boost::optional<barobo_rpc_Reply>)
        > init { std::forward<CompletionToken>(token) };

        bool success;
        typename decltype(mReplyMap)::iterator iter;
        auto&& replyElement = std::make_pair(requestId, TimedReply(mMessageQueue.get_io_service()));
        std::tie(iter, success) = mReplyMap.insert(
            std::make_pair(requestId, TimedReply{mMessageQueue.get_io_service()}));
        assert(success);

        auto self = this->shared_from_this();
        iter->second.timer->expires_from_now(timeout);
        iter->second.timer->async_wait(
            std::bind(&ClientImpl::handleReply, self, requestId, _1, boost::none));
        iter->second.queue.consume(std::move(init.handler));
        startReceivePump();

        return init.result.get();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code, barobo_rpc_Broadcast))
    asyncReceiveBroadcast (Handler&& handler) {
        util::asio::AsyncCompletion<
            Handler, void(boost::system::error_code, barobo_rpc_Broadcast)
        > init { std::forward<Handler>(handler) };

        mBroadcastQueue.consume(std::move(init.handler));
        startReceivePump();

        return init.result.get();
    }

    void handleReply (RequestId requestId,
            boost::system::error_code ec, boost::optional<barobo_rpc_Reply> reply) {
        auto iter = mReplyMap.find(requestId);
        if (mReplyMap.cend() != iter) {
            auto& elem = iter->second;
            elem.timer->cancel();
            elem.queue.produce(ec, reply);
            mReplyMap.erase(iter);
        }
        else if (reply) {
            using boost::log::add_value;
            using std::to_string;
            BOOST_LOG(mLog) << add_value("RequestId", to_string(requestId)) << "unsolicited reply";
        }
    }

    struct ReceivePumpOperation;

    void startReceivePump () {
        if (mReceivePumpRunning) {
            return;
        }
        mReceivePumpRunning = true;
        mReceivePumpError = {};

        auto self = this->shared_from_this();
        auto handler = [self, this](boost::system::error_code ec) {
            mReceivePumpRunning = false;
            mReceivePumpError = ec;
        };
        using Op = ReceivePumpOperation;
        util::asio::makeOperation<Op>(std::move(handler), self)();
    }

    void handleMessage (uint8_t* data, size_t size, boost::system::error_code& ec) {
        ec = {};
        auto status = rpc::Status::OK;
        barobo_rpc_ServerMessage message;
        decode(message, data, size, status);
        if (rpc::hasError(status)) { ec = status; return; }

        switch (message.type) {
            case barobo_rpc_ServerMessage_Type_REPLY:
                if (!message.has_inReplyTo || !message.has_reply) {
                    ec = Status::PROTOCOL_ERROR;
                    return;
                }
                handleReply(message.inReplyTo, boost::system::error_code(), message.reply);
                break;
            case barobo_rpc_ServerMessage_Type_BROADCAST:
                if (message.has_inReplyTo || !message.has_broadcast) {
                    ec = Status::PROTOCOL_ERROR;
                    return;
                }
                mBroadcastQueue.produce(boost::system::error_code(), message.broadcast);
                break;
            default:
                ec = Status::PROTOCOL_ERROR;
                return;
        }
    }

    void voidHandlers (boost::system::error_code ec) {
        BOOST_LOG(mLog) << "voiding all handlers with " << ec.message();
        for (auto& pair : mReplyMap) {
            pair.second.timer->cancel();
            while (pair.second.queue.depth() < 0) {
                pair.second.queue.produce(ec, boost::none);
            }
            while (pair.second.queue.depth() > 0) {
                pair.second.queue.consume([this](boost::system::error_code ec2,
                        boost::optional<barobo_rpc_Reply>) {
                    BOOST_LOG(mLog) << "RPC client discarding reply: " << ec2.message();
                });
            }
        }
        mReplyMap.clear();

        while (mBroadcastQueue.depth() < 0) {
            mBroadcastQueue.produce(ec, barobo_rpc_Broadcast());
        }
        while (mBroadcastQueue.depth() > 0) {
            mBroadcastQueue.consume([this](boost::system::error_code ec2, barobo_rpc_Broadcast) {
                BOOST_LOG(mLog) << "RPC client discarding broadcast: " << ec2.message();
            });
        }
    }

    RequestId nextRequestId () { return mNextRequestId++; }

    MessageQueue mMessageQueue;

    std::atomic<RequestId> mNextRequestId = { 0 };

    using ReplyQueue = util::ProducerConsumerQueue<boost::system::error_code,
        boost::optional<barobo_rpc_Reply>>;
    struct TimedReply {
        explicit TimedReply (boost::asio::io_service& context)
            : timer(std::make_shared<boost::asio::steady_timer>(context))
        {}
        std::shared_ptr<boost::asio::steady_timer> timer;
        // hack to make TimedReply copyable, required by mReplyMap.emplace() for some reason
        ReplyQueue queue;
    };
    std::map<RequestId, TimedReply> mReplyMap;

    util::ProducerConsumerQueue<boost::system::error_code, barobo_rpc_Broadcast> mBroadcastQueue;

    bool mReceivePumpRunning = false;
    boost::system::error_code mReceivePumpError;

    mutable boost::log::sources::logger mLog;
};


template <class MessageQueue>
struct ClientImpl<MessageQueue>::SendRequestOperation {
    using Nest = ClientImpl<MessageQueue>;
    using RequestId = typename Nest::RequestId;

    SendRequestOperation (
            std::shared_ptr<Nest> nest, RequestId requestId, barobo_rpc_Request request)
        : nest_(std::move(nest))
        , requestId_(requestId)
        , request_(request)
        , buf_(1024)
    {}

    std::shared_ptr<Nest> nest_;
    typename Nest::RequestId requestId_;
    barobo_rpc_Request request_;

    std::vector<uint8_t> buf_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}) {
        reenter (op) {
            yield {
                barobo_rpc_ClientMessage message;
                memset(&message, 0, sizeof(message));
                message.id = requestId_;
                memcpy(&message.request, &request_, sizeof(request_));
                pb_size_t bytesWritten;
                auto status = rpc::Status::OK;
                rpc::encode(message, buf_.data(), buf_.size(), bytesWritten, status);
                if (rpc::hasError(status)) {
                    rc_ = status;
                    BOOST_LOG(nest_->mLog) << "SendRequestOperation: " << rc_.message();
                    break;
                }
                buf_.resize(bytesWritten);
                nest_->mMessageQueue.asyncSend(boost::asio::buffer(buf_), std::move(op));
            }
            if (!ec) {
                using boost::log::add_value;
                using std::to_string;
                BOOST_LOG(nest_->mLog) << add_value("RequestId", to_string(requestId_))
                    << "sent request";
            }
            rc_ = ec;
        }
    }
};

template <class MessageQueue>
struct ClientImpl<MessageQueue>::ReceivePumpOperation {
    using Nest = ClientImpl<MessageQueue>;

    ReceivePumpOperation (std::shared_ptr<Nest> nest)
        : nest_(std::move(nest))
        , buf_(1024)
    {}

    std::shared_ptr<Nest> nest_;

    std::vector<uint8_t> buf_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}, size_t nBytesTransferred = 0) {
        if (!ec) reenter (op) {
            while (nest_->mReplyMap.size() || (nest_->mBroadcastQueue.depth() < 0)) {
                yield nest_->mMessageQueue.asyncReceive(boost::asio::buffer(buf_), std::move(op));
                if (nBytesTransferred) {
                    //BOOST_LOG(mLog) << "handleReceive: received " << nBytesTransferred << " bytes";
                    nest_->handleMessage(buf_.data(), nBytesTransferred, ec);
                    if (ec) {
                        rc_ = ec;
                        BOOST_LOG(nest_->mLog) << "ReceivePumpOperation: " << ec.message();
                        nest_->voidHandlers(ec);
                        yield break;
                    }
                }
                else {
                    // It's cool, just a keepalive
                }
            }
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
            BOOST_LOG(nest_->mLog) << "ReceivePumpOperation: " << ec.message();
            nest_->voidHandlers(ec);
        }
    }
};

template <class C, class Duration>
struct RequestOperation {
    RequestOperation (C& client, barobo_rpc_Request request, Duration&& timeout)
        : client_(client)
        , request_(request)
        , timeout_(std::forward<Duration>(timeout))
    {}

    C& client_;
    barobo_rpc_Request request_;
    Duration timeout_;

    typename C::RequestId requestId_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;
    boost::optional<barobo_rpc_Reply> reply_ = boost::none;

    auto result () const {
        return std::make_tuple(rc_, reply_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {},
            boost::optional<barobo_rpc_Reply> reply = {}) {
        if (!ec) reenter (op) {
            requestId_ = client_.nextRequestId();
            yield client_.asyncSendRequest(requestId_, request_, std::move(op));
            yield client_.asyncReceiveReply(requestId_, timeout_, std::move(op));
            rc_ = ec;
            reply_ = reply;
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
        }
    }
};

template <class C, class Duration, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
    void(boost::system::error_code, boost::optional<barobo_rpc_Reply>))
asyncRequest (
        C& client, barobo_rpc_Request request, Duration&& timeout, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, void(boost::system::error_code, boost::optional<barobo_rpc_Reply>)
    > init { std::forward<CompletionToken>(token) };

    using Op = RequestOperation<C, Duration>;
    util::asio::makeOperation<Op>(std::move(init.handler),
        client, request, std::forward<Duration>(timeout))();

    return init.result.get();
}

template <class MessageQueue>
class Client : public util::asio::TransparentIoObject<ClientImpl<MessageQueue>> {
public:
    using RequestId = typename ClientImpl<MessageQueue>::RequestId;

    explicit Client (boost::asio::io_service& context)
        : util::asio::TransparentIoObject<ClientImpl<MessageQueue>>(context)
    {}

    MessageQueue& messageQueue () {
        return this->get_implementation()->messageQueue();
    }
    const MessageQueue& messageQueue () const {
        return this->get_implementation()->messageQueue();
    }

    boost::log::sources::logger& log () {
        return this->get_implementation()->log();
    }

    RequestId nextRequestId () {
        return this->get_implementation()->nextRequestId();
    }

    UTIL_ASIO_DECL_ASYNC_METHOD(asyncSendRequest)
    UTIL_ASIO_DECL_ASYNC_METHOD(asyncReceiveReply)
    UTIL_ASIO_DECL_ASYNC_METHOD(asyncReceiveBroadcast)
};

// Make a disconnection request to the remote server.
template <class RpcClient, class Duration, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncDisconnect (RpcClient& client, Duration&& timeout, Handler&& handler) {
    util::asio::AsyncCompletion<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    auto log = client.log();

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));
    request.type = barobo_rpc_Request_Type_DISCONNECT;
    //BOOST_LOG(log) << "sending DISCONNECT request";
    asyncRequest(client, request, std::forward<Duration>(timeout),
        [realHandler, log] (boost::system::error_code ec,
                boost::optional<barobo_rpc_Reply> reply) mutable {
            if (ec) {
                BOOST_LOG(log) << "DISCONNECT request completed with error: " << ec.message();
                realHandler(ec);
                return;
            }
            else if (!reply) {
                BOOST_LOG(log) << "DISCONNECT request timed out";
                realHandler(Status::TIMED_OUT);
                return;
            }
            switch (reply->type) {
                case barobo_rpc_Reply_Type_VERSIONS:
                    BOOST_LOG(log) << "DISCONNECT request completed with VERSIONS (inconsistent reply)";
                    realHandler(Status::PROTOCOL_ERROR);
                    break;
                case barobo_rpc_Reply_Type_STATUS:
                    if (!reply->has_status) {
                        BOOST_LOG(log) << "DISCONNECT request completed with inconsistent STATUS reply";
                        realHandler(Status::PROTOCOL_ERROR);
                    }
                    else {
                        auto remoteEc = make_error_code(RemoteStatus(reply->status.value));
                        BOOST_LOG(log) << "DISCONNECT request completed with STATUS: " << remoteEc.message();
                        realHandler(remoteEc);
                    }
                    break;
                case barobo_rpc_Reply_Type_RESULT:
                    BOOST_LOG(log) << "DISCONNECT request completed with RESULT (inconsistent reply)";
                    realHandler(Status::PROTOCOL_ERROR);
                    break;
                default:
                    BOOST_LOG(log) << "DISCONNECT request completed with unrecognized reply type";
                    realHandler(Status::PROTOCOL_ERROR);
                    break;
            }
        });

    return init.result.get();
}

// Make a connection request to the remote server, error on version mismatch.
// FIXME I think it probably makes more sense for the client to pass version
// information to the server, and let the server reject based on version
// mismatch. This is less flexible (i.e., harder for the client to implement
// multiple versions and adapt whatever the server's offering), but that
// flexibility is unlikely to be useful any time soon, and if we make the
// server in charge of rejecting version mismatches, we can guarantee that we
// won't accidentally leave the server in a connected state.
template <class Interface, class RpcClient, class Duration, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncConnect (RpcClient& client, Duration&& timeout, Handler&& handler) {
    util::asio::AsyncCompletion<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    auto log = client.log();

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));
    request.type = barobo_rpc_Request_Type_CONNECT;
    BOOST_LOG(log) << "sending CONNECT request";
    asyncRequest(client, request, std::forward<Duration>(timeout),
        [&client, timeout, realHandler, log] (boost::system::error_code ec,
                boost::optional<barobo_rpc_Reply> reply) mutable {
            auto& ios = client.get_io_service();
            if (ec) {
                BOOST_LOG(log) << "CONNECT request completed with error: " << ec.message();
                ios.post(std::bind(realHandler, ec));
                return;
            }
            else if (!reply) {
                BOOST_LOG(log) << "CONNECT request timed out";
                ios.post(std::bind(realHandler, Status::TIMED_OUT));
                return;
            }
            switch (reply->type) {
                case barobo_rpc_Reply_Type_VERSIONS:
                    if (!reply->has_versions) {
                        BOOST_LOG(log) << "CONNECT request completed with inconsistent VERSIONS reply";
                        ios.post(std::bind(realHandler, Status::PROTOCOL_ERROR));
                    }
                    else {
                        BOOST_LOG(log) << "CONNECT request completed with VERSIONS (success)";
                        auto vers = Versions{reply->versions};

                        BOOST_LOG(log) << "Remote RPC version " << vers.rpc()
                                       << ", interface version " << vers.interface();
                        BOOST_LOG(log) << "Local RPC version " << rpc::Version<>::triplet()
                                       << ", interface version " << rpc::Version<Interface>::triplet();

                        if (vers.rpc() != rpc::Version<>::triplet() ||
                            vers.interface() != rpc::Version<Interface>::triplet()) {
                            asyncDisconnect(client, timeout, [&ios, realHandler] (boost::system::error_code) {
                                ios.post(std::bind(realHandler, Status::VERSION_MISMATCH));
                            });
                        }
                        else {
                            ios.post(std::bind(realHandler, Status::OK));
                        }
                    }
                    break;
                case barobo_rpc_Reply_Type_STATUS:
                    if (!reply->has_status || barobo_rpc_Status_OK == reply->status.value) {
                        BOOST_LOG(log) << "CONNECT request completed with inconsistent STATUS reply";
                        ios.post(std::bind(realHandler, Status::PROTOCOL_ERROR));
                    }
                    else {
                        auto remoteEc = make_error_code(RemoteStatus(reply->status.value));
                        BOOST_LOG(log) << "CONNECT request completed with STATUS: " << remoteEc.message();
                        ios.post(std::bind(realHandler, remoteEc));
                    }
                    break;
                case barobo_rpc_Reply_Type_RESULT:
                    BOOST_LOG(log) << "CONNECT request completed with RESULT (inconsistent reply)";
                    ios.post(std::bind(realHandler, Status::PROTOCOL_ERROR));
                    break;
                default:
                    BOOST_LOG(log) << "CONNECT request completed with unrecognized reply type";
                    ios.post(std::bind(realHandler, Status::PROTOCOL_ERROR));
                    break;
            }
        });

    return init.result.get();
}

// Make a fire method request to the remote server.
template <class RpcClient, class Method, class Duration, class Handler, class Result = typename ResultOf<Method>::type>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code, Result))
asyncFire (RpcClient& client, Method args, Duration&& timeout, Handler&& handler) {
    util::asio::AsyncCompletion<
        Handler, void(boost::system::error_code, Result)
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    auto log = client.log();

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));
    request.type = barobo_rpc_Request_Type_FIRE;
    request.has_fire = true;
    request.fire.id = componentId(args);
    Status status;
    rpc::encode(args,
        request.fire.payload.bytes,
        sizeof(request.fire.payload.bytes),
        request.fire.payload.size, status);
    if (hasError(status)) {
        auto encodingEc = make_error_code(status);
        BOOST_LOG(log) << "FIRE request failed to encode: " << encodingEc.message();
        client.get_io_service().post(std::bind(realHandler, encodingEc, Result()));
    }
    else {
        BOOST_LOG(log) << "sending FIRE request";
        asyncRequest(client, request, std::forward<Duration>(timeout),
            [realHandler, log] (boost::system::error_code ec,
                    boost::optional<barobo_rpc_Reply> reply) mutable {
                if (ec) {
                    BOOST_LOG(log) << "FIRE request completed with error: " << ec.message();
                    realHandler(ec, Result());
                    return;
                }
                else if (!reply) {
                    BOOST_LOG(log) << "FIRE request timed out";
                    realHandler(Status::TIMED_OUT, Result());
                    return;
                }
                switch (reply->type) {
                    case barobo_rpc_Reply_Type_VERSIONS:
                        BOOST_LOG(log) << "FIRE request completed with VERSIONS (inconsistent reply)";
                        realHandler(Status::PROTOCOL_ERROR, Result());
                        break;
                    case barobo_rpc_Reply_Type_STATUS:
                        if (!reply->has_status) {
                            BOOST_LOG(log) << "FIRE request completed with inconsistent STATUS reply";
                            realHandler(Status::PROTOCOL_ERROR, Result());
                        }
                        else {
                            auto remoteEc = make_error_code(RemoteStatus(reply->status.value));
                            BOOST_LOG(log) << "FIRE request completed with STATUS: " << remoteEc.message();
                            realHandler(remoteEc, Result());
                        }
                        break;
                    case barobo_rpc_Reply_Type_RESULT:
                        if (!reply->has_result) {
                            BOOST_LOG(log) << "FIRE request completed with inconsistent RESULT reply";
                            realHandler(Status::PROTOCOL_ERROR, Result());
                        }
                        else {
                            Status status;
                            Result result;
                            memset(&result, 0, sizeof(result));
                            rpc::decode(result, reply->result.payload.bytes, reply->result.payload.size, status);
                            auto decodingEc = make_error_code(status);
                            BOOST_LOG(log) << "FIRE request completed with RESULT (decoding status: "
                                           << decodingEc.message() << ")";
                            realHandler(decodingEc, result);
                        }
                        break;
                    default:
                        BOOST_LOG(log) << "FIRE request completed with unrecognized reply type";
                        realHandler(Status::PROTOCOL_ERROR, Result());
                        break;
                }
            });
    }

    return init.result.get();
}

template <class Interface, class C, class Impl>
struct RunClientOperation {
    RunClientOperation (C& client, Impl& impl)
        : client_(client)
        , impl_(impl)
    {}

    C& client_;
    Impl& impl_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    auto result () {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {},
            barobo_rpc_Broadcast broadcast = {}) {
        if (!ec) reenter (op) {
            yield client_.asyncReceiveBroadcast(std::move(op));
            while (1) {
                BOOST_LOG(client_.log()) << "broadcast received";
                yield {
                    rpc::BroadcastUnion<Interface> b;
                    rpc::Status status;
                    b.invoke(impl_, broadcast.id, broadcast.payload, status);
                    if (hasError(status)) {
                        rc_ = status;
                        BOOST_LOG(client_.log())
                            << "RunClientOperation: broadcast invocation error: " << ec.message();
                        break;
                    }
                    else {
                        client_.asyncReceiveBroadcast(std::move(op));
                    }
                }
            }
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
        }
    }
};

template <class Interface, class C, class Impl, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncRunClient (C& client, Impl& impl, Handler&& handler) {
    util::asio::AsyncCompletion<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };

    using Op = RunClientOperation<Interface, C, Impl>;
    util::asio::makeOperation<Op>(std::move(init.handler), client, impl)();

    return init.result.get();
}

}} // namespace rpc::asio

#include <boost/asio/unyield.hpp>

#endif
