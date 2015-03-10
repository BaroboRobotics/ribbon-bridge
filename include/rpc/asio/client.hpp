#ifndef RPC_ASIO_CLIENT_HPP
#define RPC_ASIO_CLIENT_HPP

#include "rpc/componenttraits.hpp"
#include "rpc/message.hpp"
#include "rpc/system_error.hpp"
#include "rpc/version.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include <boost/unordered_map.hpp>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <atomic>
#include <memory>
#include <queue>
#include <utility>

namespace rpc {
namespace asio {

using namespace std::placeholders;

template <class MessageQueue>
class Client {
    using RequestId = uint32_t;
public:

    template <class... Args>
    explicit Client (Args&&... args)
        : mImpl(std::make_shared<Impl>(std::forward<Args>(args)...))
    {}

    ~Client () {
        boost::system::error_code ec;
        close(ec);
    }

    void close () {
        boost::system::error_code ec;
        close(ec);
        if (ec) {
            throw boost::system::system_error(ec);
        }
    }

    void close (boost::system::error_code& ec) {
        mImpl->close(ec);
    }

    boost::log::sources::logger log () { return mImpl->mLog; }

    boost::asio::io_service& get_io_service () { return mImpl->mMessageQueue.get_io_service(); }

    MessageQueue& messageQueue () { return mImpl->mMessageQueue; }
    const MessageQueue& messageQueue () const { return mImpl->mMessageQueue; }

    // Make a request of the remote server and wait for the reply to arrive.
    // In general, you'll want to use the asyncConnect, asyncDisconnect, and
    // asyncFire free functions instead, to make this easier.
    template <class Duration, class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code,barobo_rpc_Reply))
    asyncRequest (barobo_rpc_Request request, Duration&& timeout, Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, void(boost::system::error_code,barobo_rpc_Reply)
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        auto m = mImpl;
        auto requestId = m->nextRequestId();

        barobo_rpc_ClientMessage message;
        memset(&message, 0, sizeof(message));
        message.id = requestId;
        memcpy(&message.request, &request, sizeof(request));

        auto buf = std::make_shared<std::vector<uint8_t>>(1024);
        try {
            pb_size_t bytesWritten;
            rpc::encode(message, buf->data(), buf->size(), bytesWritten);
            buf->resize(bytesWritten);

            m->mStrand.post([m, buf, requestId, realHandler, timeout] () {
                using boost::log::add_value;
                using std::to_string;
                BOOST_LOG(m->mLog) << add_value("RequestId", to_string(requestId)) << "beginning transaction";
                m->emplaceReplyHandler(requestId, realHandler);
                m->emplaceReplyTimeout(requestId, timeout);

                m->mMessageQueue.asyncSend(boost::asio::buffer(*buf),
                    m->mStrand.wrap([m, buf, requestId] (boost::system::error_code ec) {
                        if (ec) {
                            m->handleReply(requestId, ec, barobo_rpc_Reply());
                        }
                    }));
            });
        }
        catch (boost::system::system_error& e) {
            BOOST_LOG(m->mLog) << "client: posting error in asyncRequest";
            m->mMessageQueue.get_io_service().post(std::bind(realHandler, e.code(), barobo_rpc_Reply()));
        }

        return init.result.get();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code,barobo_rpc_Broadcast))
    asyncReceiveBroadcast (Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, void(boost::system::error_code,barobo_rpc_Broadcast)
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        auto m = mImpl;
        m->mStrand.post([m, realHandler] () {
            m->mBroadcastHandlers.push(realHandler);
            m->postBroadcasts();
            m->startReceiveCoroutine();
        });

        return init.result.get();
    }

private:
    using ReplyHandler = std::function<void(boost::system::error_code,barobo_rpc_Reply)>;
    using BroadcastHandler = std::function<void(boost::system::error_code,barobo_rpc_Broadcast)>;

    // Impl's raison d'etre is the same as that detailed in sfp::asio::MessageQueue.
    // Whose comment I just removed. Here it is:
    // Since a MessageQueue can post handlers which might run after the
    // MessageQueue is destroyed, all data members must be wrapped in a struct
    // to be managed by a std::shared_ptr. All handlers must then have copy of
    // this shared_ptr, either in their lambda capture list (as in the 'm' in
    // MessageQueue's async* operations) or stored in their std::bind-created
    // function object (as in the calls to this->shared_from_this() in
    // handshakeCoroutine). This guarantees that all handlers/coroutines run
    // with access to a valid this pointer.
    struct Impl : std::enable_shared_from_this<Impl> {
        explicit Impl (boost::asio::io_service& ios, boost::log::sources::logger log)
            : mMessageQueue(ios, log)
            , mIoService(mMessageQueue.get_io_service())
            , mStrand(mIoService)
            , mLog(log)
        {
            mLog.add_attribute("Protocol", boost::log::attributes::constant<std::string>("RB-CL"));
        }

        void close (boost::system::error_code& ec) {
            mMessageQueue.close(ec);
        }

        void handleReply (RequestId requestId, boost::system::error_code ec, barobo_rpc_Reply reply) {
            auto handlerIter = mReplyHandlers.find(requestId);
            if (mReplyHandlers.cend() != handlerIter) {
                using boost::log::add_value;
                using std::to_string;
                BOOST_LOG(mLog) << add_value("RequestId", to_string(requestId))
                                << "Posting reply handler with " << ec.message();
                mIoService.post(std::bind(handlerIter->second, ec, reply));
                mReplyHandlers.erase(handlerIter);
            }
            else {
                using boost::log::add_value;
                using std::to_string;
                BOOST_LOG(mLog) << add_value("RequestId", to_string(requestId)) << "unsolicited reply";
            }

            auto timeoutIter = mReplyTimeouts.find(requestId);
            if (mReplyTimeouts.cend() != timeoutIter) {
                timeoutIter->second.cancel();
                mReplyTimeouts.erase(timeoutIter);
            }
        }

        void emplaceReplyHandler (RequestId requestId, ReplyHandler replyHandler) {
            mReplyHandlers.emplace(std::piecewise_construct,
                std::forward_as_tuple(requestId),
                std::forward_as_tuple(replyHandler));
            startReceiveCoroutine();
            using boost::log::add_value;
            using std::to_string;
        }

        template <class Duration>
        void emplaceReplyTimeout (RequestId requestId, Duration&& timeout) {
            auto& timer = mReplyTimeouts.emplace(std::piecewise_construct,
                std::forward_as_tuple(requestId),
                std::forward_as_tuple(mIoService)).first->second;
            timer.expires_from_now(timeout);
            auto m = this->shared_from_this();
            using boost::log::add_value;
            using std::to_string;
            timer.async_wait(mStrand.wrap([m, requestId] (boost::system::error_code ec) {
                if (!ec) {
                    BOOST_LOG(m->mLog) << add_value("RequestId", to_string(requestId)) << "timed out";
                    m->handleReply(requestId, Status::TIMED_OUT, barobo_rpc_Reply());
                }
            }));
        }

        void postReplies () {
            while (mReplyInbox.size()) {
                auto& reply = mReplyInbox.front();
                handleReply(reply.first, boost::system::error_code(), reply.second);
                mReplyInbox.pop();
            }
        }

        void postBroadcasts () {
            while (mBroadcastInbox.size() && mBroadcastHandlers.size()) {
                auto& broadcast = mBroadcastInbox.front();
                auto& handler = mBroadcastHandlers.front();
                mIoService.post(std::bind(handler, boost::system::error_code(), broadcast));
                mBroadcastInbox.pop();
                mBroadcastHandlers.pop();
            }
        }

        void startReceiveCoroutine () {
            if (1 == (mReplyHandlers.size() + mBroadcastHandlers.size())) {
                auto buf = std::make_shared<std::vector<uint8_t>>(1024);
                receive(buf);
            }
        }

        void receive (std::shared_ptr<std::vector<uint8_t>> buf) {
            if (mReplyHandlers.size() || mBroadcastHandlers.size()) {
                mMessageQueue.asyncReceive(boost::asio::buffer(*buf), mStrand.wrap(
                    std::bind(&Client::Impl::handleReceive,
                        this->shared_from_this(), buf, _1, _2)));
            }
        }

        void handleReceive (std::shared_ptr<std::vector<uint8_t>> buf,
                            boost::system::error_code ec,
                            size_t nBytesTransferred) {
            try {
                if (ec) {
                    throw boost::system::system_error(ec);
                }

                if (nBytesTransferred) {
                    //BOOST_LOG(mLog) << "handleReceive: received " << nBytesTransferred << " bytes";
                    barobo_rpc_ServerMessage message;
                    decode(message, buf->data(), nBytesTransferred);

                    switch (message.type) {
                        case barobo_rpc_ServerMessage_Type_REPLY:
                            if (!message.has_inReplyTo || !message.has_reply) {
                                // FIXME INCONSISTENT_REPLY should be INCONSISTENT_MESSAGE
                                throw Error(Status::INCONSISTENT_REPLY);
                            }
                            mReplyInbox.push(std::make_pair(message.inReplyTo, message.reply));
                            break;
                        case barobo_rpc_ServerMessage_Type_BROADCAST:
                            if (message.has_inReplyTo || !message.has_broadcast) {
                                throw Error(Status::INCONSISTENT_REPLY);
                            }
                            mBroadcastInbox.push(message.broadcast);
                            break;
                        default:
                            throw Error(Status::INCONSISTENT_REPLY);
                            break;
                    }

                    postReplies();
                    postBroadcasts();
                }
                else {
                    // It's cool, just a keepalive
                }
                receive(buf);
            }
            catch (boost::system::system_error& e) {
                BOOST_LOG(mLog) << "handleReceive: " << e.what();
                voidHandlers(e.code());
            }
        }

        void voidHandlers (boost::system::error_code ec) {
            BOOST_LOG(mLog) << "voiding all handlers with " << ec.message();
            for (auto& pair : mReplyHandlers) {
                mIoService.post(std::bind(pair.second, ec, barobo_rpc_Reply()));
            }
            mReplyHandlers.clear();
            for (auto& pair: mReplyTimeouts) {
                pair.second.cancel();
            }
            mReplyTimeouts.clear();
            while (mBroadcastHandlers.size()) {
                mIoService.post(std::bind(mBroadcastHandlers.front(), ec, barobo_rpc_Broadcast()));
                mBroadcastHandlers.pop();
            }
        }

        RequestId nextRequestId () { return mNextRequestId++; }

        MessageQueue mMessageQueue;
        boost::asio::io_service& mIoService;
        boost::asio::io_service::strand mStrand;

        std::atomic<RequestId> mNextRequestId = { 0 };

        std::queue<std::pair<RequestId, barobo_rpc_Reply>> mReplyInbox;
        boost::unordered_map<RequestId, ReplyHandler> mReplyHandlers;
        boost::unordered_map<RequestId, boost::asio::steady_timer> mReplyTimeouts;

        std::queue<barobo_rpc_Broadcast> mBroadcastInbox;
        std::queue<BroadcastHandler> mBroadcastHandlers;

        mutable boost::log::sources::logger mLog;
    };


    std::shared_ptr<Impl> mImpl;
};

// Make a disconnection request to the remote server.
template <class RpcClient, class Duration, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncDisconnect (RpcClient& client, Duration&& timeout, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    auto log = client.log();

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));
    request.type = barobo_rpc_Request_Type_DISCONNECT;
    BOOST_LOG(log) << "sending DISCONNECT request";
    client.asyncRequest(request, std::forward<Duration>(timeout),
        [realHandler, log] (boost::system::error_code ec, barobo_rpc_Reply reply) mutable {
            if (ec) {
                BOOST_LOG(log) << "DISCONNECT request completed with error: " << ec.message();
                realHandler(ec);
                return;
            }
            switch (reply.type) {
                case barobo_rpc_Reply_Type_SERVICEINFO:
                    BOOST_LOG(log) << "DISCONNECT request completed with SERVICEINFO (inconsistent reply)";
                    realHandler(Status::INCONSISTENT_REPLY);
                    break;
                case barobo_rpc_Reply_Type_STATUS:
                    if (!reply.has_status) {
                        BOOST_LOG(log) << "DISCONNECT request completed with inconsistent STATUS reply";
                        realHandler(Status::INCONSISTENT_REPLY);
                    }
                    else {
                        auto remoteEc = make_error_code(RemoteStatus(reply.status.value));
                        BOOST_LOG(log) << "DISCONNECT request completed with STATUS: " << remoteEc.message();
                        realHandler(remoteEc);
                    }
                    break;
                case barobo_rpc_Reply_Type_RESULT:
                    BOOST_LOG(log) << "DISCONNECT request completed with RESULT (inconsistent reply)";
                    realHandler(Status::INCONSISTENT_REPLY);
                    break;
                default:
                    BOOST_LOG(log) << "DISCONNECT request completed with unrecognized reply type";
                    realHandler(Status::INCONSISTENT_REPLY);
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
    boost::asio::detail::async_result_init<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    auto log = client.log();

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));
    request.type = barobo_rpc_Request_Type_CONNECT;
    BOOST_LOG(log) << "sending CONNECT request";
    client.asyncRequest(request, std::forward<Duration>(timeout),
        [&client, timeout, realHandler, log] (boost::system::error_code ec, barobo_rpc_Reply reply) mutable {
            auto& ios = client.get_io_service();
            if (ec) {
                BOOST_LOG(log) << "CONNECT request completed with error: " << ec.message();
                ios.post(std::bind(realHandler, ec));
                return;
            }
            switch (reply.type) {
                case barobo_rpc_Reply_Type_SERVICEINFO:
                    if (!reply.has_serviceInfo) {
                        BOOST_LOG(log) << "CONNECT request completed with inconsistent SERVICEINFO reply";
                        ios.post(std::bind(realHandler, Status::INCONSISTENT_REPLY));
                    }
                    else {
                        BOOST_LOG(log) << "CONNECT request completed with SERVICEINFO (success)";
                        auto info = ServiceInfo{reply.serviceInfo};

                        BOOST_LOG(log) << "Remote RPC version " << info.rpcVersion()
                                       << ", interface version " << info.interfaceVersion();
                        BOOST_LOG(log) << "Local RPC version " << rpc::Version<>::triplet()
                                       << ", interface version " << rpc::Version<Interface>::triplet();

                        if (info.rpcVersion() != rpc::Version<>::triplet() ||
                            info.interfaceVersion() != rpc::Version<Interface>::triplet()) {
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
                    if (!reply.has_status || barobo_rpc_Status_OK == reply.status.value) {
                        BOOST_LOG(log) << "CONNECT request completed with inconsistent STATUS reply";
                        ios.post(std::bind(realHandler, Status::INCONSISTENT_REPLY));
                    }
                    else {
                        auto remoteEc = make_error_code(RemoteStatus(reply.status.value));
                        BOOST_LOG(log) << "CONNECT request completed with STATUS: " << remoteEc.message();
                        ios.post(std::bind(realHandler, remoteEc));
                    }
                    break;
                case barobo_rpc_Reply_Type_RESULT:
                    BOOST_LOG(log) << "CONNECT request completed with RESULT (inconsistent reply)";
                    ios.post(std::bind(realHandler, Status::INCONSISTENT_REPLY));
                    break;
                default:
                    BOOST_LOG(log) << "CONNECT request completed with unrecognized reply type";
                    ios.post(std::bind(realHandler, Status::INCONSISTENT_REPLY));
                    break;
            }
        });

    return init.result.get();
}

// Make a fire method request to the remote server.
template <class RpcClient, class Method, class Duration, class Handler, class Result = typename ResultOf<Method>::type>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code, Result))
asyncFire (RpcClient& client, Method args, Duration&& timeout, Handler&& handler) {
    boost::asio::detail::async_result_init<
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
        client.asyncRequest(request, std::forward<Duration>(timeout),
            [realHandler, log] (boost::system::error_code ec, barobo_rpc_Reply reply) mutable {
                if (ec) {
                    BOOST_LOG(log) << "FIRE request completed with error: " << ec.message();
                    realHandler(ec, Result());
                    return;
                }
                switch (reply.type) {
                    case barobo_rpc_Reply_Type_SERVICEINFO:
                        BOOST_LOG(log) << "FIRE request completed with SERVICEINFO (inconsistent reply)";
                        realHandler(Status::INCONSISTENT_REPLY, Result());
                        break;
                    case barobo_rpc_Reply_Type_STATUS:
                        if (!reply.has_status) {
                            BOOST_LOG(log) << "FIRE request completed with inconsistent STATUS reply";
                            realHandler(Status::INCONSISTENT_REPLY, Result());
                        }
                        else {
                            auto remoteEc = make_error_code(RemoteStatus(reply.status.value));
                            BOOST_LOG(log) << "FIRE request completed with STATUS: " << remoteEc.message();
                            realHandler(remoteEc, Result());
                        }
                        break;
                    case barobo_rpc_Reply_Type_RESULT:
                        if (!reply.has_result) {
                            BOOST_LOG(log) << "FIRE request completed with inconsistent RESULT reply";
                            realHandler(Status::INCONSISTENT_REPLY, Result());
                        }
                        else {
                            Status status;
                            Result result;
                            memset(&result, 0, sizeof(result));
                            rpc::decode(result, reply.result.payload.bytes, reply.result.payload.size, status);
                            auto decodingEc = make_error_code(status);
                            BOOST_LOG(log) << "FIRE request completed with RESULT (decoding status: "
                                           << decodingEc.message() << ")";
                            realHandler(decodingEc, result);
                        }
                        break;
                    default:
                        BOOST_LOG(log) << "FIRE request completed with unrecognized reply type";
                        realHandler(Status::INCONSISTENT_REPLY, Result());
                        break;
                }
            });
    }

    return init.result.get();
}

template <class Interface, class C, class Impl, class Handler>
struct RunClientOperation : std::enable_shared_from_this<RunClientOperation<Interface, C, Impl, Handler>> {
    RunClientOperation (C& client, Impl& impl)
        : mIos(client.get_io_service())
        , mStrand(mIos)
        , mClient(client)
        , mImpl(impl)
    {}

    void start (Handler handler) {
        mClient.asyncReceiveBroadcast(mStrand.wrap(
            std::bind(&RunClientOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (Handler handler, boost::system::error_code ec, barobo_rpc_Broadcast broadcast) {
        try {
            if (ec) {
                throw boost::system::system_error(ec);
            }

            auto log = mClient.log();
            BOOST_LOG(log) << "broadcast received";
            rpc::BroadcastUnion<Interface> b;
            rpc::Status status;
            b.invoke(mImpl, broadcast.id, broadcast.payload, status);
            if (hasError(status)) {
                ec = status;
                BOOST_LOG(log) << "RunClientOperation: broadcast invocation error: " << ec.message();
                throw boost::system::system_error(ec);
            }

            start(handler);
        }
        catch (boost::system::system_error& e) {
            mIos.post(std::bind(handler, e.code()));
        }
    }

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    C& mClient;
    Impl& mImpl;
};

template <class Interface, class C, class Impl, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncRunClient (C& client, Impl& impl, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };

    using Op = RunClientOperation<Interface, C, Impl, decltype(init.handler)>;
    std::make_shared<Op>(client, impl)->start(init.handler);

    return init.result.get();
}

} // namespace asio
} // namespace rpc

#endif