#ifndef RPC_ASIO_SERVER_HPP
#define RPC_ASIO_SERVER_HPP

#include "rpc.pb.hpp"

#include <util/log.hpp>
#include <util/asio/asynccompletion.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <boost/log/attributes/constant.hpp>

#include <functional>
#include <utility>

#include <boost/asio/yield.hpp>

namespace rpc {
namespace asio {

template <class MessageQueue>
class Server {
public:
    using RequestId = uint32_t;
    struct RequestPair {
        RequestId id;
        barobo_rpc_Request request;
    };

    typedef void RequestHandlerSignature(boost::system::error_code, RequestPair);
    using RequestHandler = std::function<RequestHandlerSignature>;

    explicit Server (boost::asio::io_service& ios)
            : mMessageQueue(ios)
    {
        mLog.add_attribute("Protocol", boost::log::attributes::constant<std::string>("RB-SV"));
    }

    Server (Server&& that)
        : mMessageQueue(std::move(that.mMessageQueue))
        , mLog(that.mLog)
    {}

    void close () {
        boost::system::error_code ec;
        close(ec);
        if (ec) {
            throw boost::system::system_error(ec);
        }
    }

    void close (boost::system::error_code& ec) {
        mMessageQueue.close(ec);
    }

    boost::asio::io_service& get_io_service () { return mMessageQueue.get_io_service(); }

    MessageQueue& messageQueue () { return mMessageQueue; }
    const MessageQueue& messageQueue () const { return mMessageQueue; }

    util::log::Logger& log () { return mLog; }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, RequestHandlerSignature)
    asyncReceiveRequest (Handler&& handler) {
        util::asio::AsyncCompletion<
            Handler, RequestHandlerSignature
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        auto buf = std::make_shared<std::vector<uint8_t>>(1024);
        mMessageQueue.asyncReceive(boost::asio::buffer(*buf),
            [this, realHandler, buf] (boost::system::error_code ec, size_t size) mutable {
                if (!ec) {
                    if (size) {
                        barobo_rpc_ClientMessage message;
                        Status status;
                        rpc::decode(message, buf->data(), size, status);
                        this->mMessageQueue.get_io_service().post(
                            std::bind(realHandler, status, RequestPair{message.id, message.request}));
                    }
                    else {
                        // it's cool, just a keepalive
                    }
                }
                else {
                    this->mMessageQueue.get_io_service().post(
                        std::bind(realHandler, ec, RequestPair()));
                }
            });

        return init.result.get();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
    asyncSendReply (RequestId requestId, barobo_rpc_Reply reply, Handler&& handler) {
        util::asio::AsyncCompletion<
            Handler, void(boost::system::error_code)
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        barobo_rpc_ServerMessage message;
        memset(&message, 0, sizeof(message));
        message.type = barobo_rpc_ServerMessage_Type_REPLY;
        message.has_reply = true;
        memcpy(&message.reply, &reply, sizeof(reply));
        message.has_inReplyTo = true;
        message.inReplyTo = requestId;
        message.has_broadcast = false;

        auto buf = std::make_shared<std::vector<uint8_t>>(1024);
        try {
            pb_size_t bytesWritten;
            rpc::encode(message, buf->data(), buf->size(), bytesWritten);
            buf->resize(bytesWritten);
            mMessageQueue.asyncSend(boost::asio::buffer(*buf),
                [buf, realHandler] (boost::system::error_code ec) mutable {
                    realHandler(ec);
                });
        }
        catch (boost::system::system_error& e) {
            BOOST_LOG(mLog) << "error sending reply";
            mMessageQueue.get_io_service().post(std::bind(realHandler, e.code()));
        }

        return init.result.get();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
    asyncSendBroadcast (barobo_rpc_Broadcast broadcast, Handler&& handler) {
        util::asio::AsyncCompletion<
            Handler, void(boost::system::error_code)
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        barobo_rpc_ServerMessage message;
        memset(&message, 0, sizeof(message));
        message.type = barobo_rpc_ServerMessage_Type_BROADCAST;
        message.has_reply = false;
        message.has_inReplyTo = false;
        message.has_broadcast = true;
        memcpy(&message.broadcast, &broadcast, sizeof(broadcast));

        auto buf = std::make_shared<std::vector<uint8_t>>(1024);
        try {
            pb_size_t bytesWritten;
            rpc::encode(message, buf->data(), buf->size(), bytesWritten);
            buf->resize(bytesWritten);
            mMessageQueue.asyncSend(boost::asio::buffer(*buf),
                [buf, realHandler] (boost::system::error_code ec) mutable {
                    realHandler(ec);
                });
        }
        catch (boost::system::system_error& e) {
            BOOST_LOG(mLog) << "error sending broadcast";
            mMessageQueue.get_io_service().post(std::bind(realHandler, e.code()));
        }

        return init.result.get();
    }

private:
    MessageQueue mMessageQueue;

    util::log::Logger mLog;
};

template <class S, class Broadcast, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncBroadcast (S& server, Broadcast args, Handler&& handler) {
    util::asio::AsyncCompletion<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    barobo_rpc_Broadcast broadcast;
    broadcast = decltype(broadcast)();
    broadcast.id = componentId(args);
    Status status;
    rpc::encode(args,
        broadcast.payload.bytes,
        sizeof(broadcast.payload.bytes),
        broadcast.payload.size, status);
    if (hasError(status)) {
        server.get_io_service().post(std::bind(realHandler, status));
    }
    else {
        server.asyncSendBroadcast(broadcast, realHandler);
    }

    return init.result.get();
}

template <class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncReply (S& server, typename S::RequestId requestId, Status status, Handler&& handler) {
    barobo_rpc_Reply reply;
    reply = decltype(reply)();
    reply.type = barobo_rpc_Reply_Type_STATUS;
    reply.has_status = true;
    reply.status.value = decltype(reply.status.value)(status);
    server.asyncSendReply(requestId, reply, std::forward<Handler>(handler));
}

template <class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncReply (S& server, typename S::RequestId requestId, Versions vers, Handler&& handler) {
    barobo_rpc_Reply reply;
    reply = decltype(reply)();
    reply.type = barobo_rpc_Reply_Type_VERSIONS;
    reply.has_versions = true;
    reply.versions = vers;
    server.asyncSendReply(requestId, reply, std::forward<Handler>(handler));
}

template <class Interface, class S, class Impl>
struct ServeUntilDisconnectionOperation {
    using RequestPair = typename S::RequestPair;

    ServeUntilDisconnectionOperation (S& server, Impl& impl) : server_(server), impl_(impl) {}

    S& server_;
    Impl& impl_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;
    RequestPair rp_;

    auto result () {
        return std::make_tuple(rc_, rp_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}, RequestPair rp = {}) {
        if (!ec) reenter (op) {
            while (1) {
                yield server_.asyncReceiveRequest(std::move(op));
                if (barobo_rpc_Request_Type_DISCONNECT == rp.request.type) {
                    rc_ = ec;
                    rp_ = rp;
                    yield break;
                }
                else if (barobo_rpc_Request_Type_CONNECT == rp.request.type) {
                    yield asyncReply(
                        server_, rp.id, Versions::create<Interface>(), std::move(op));
                }
                else if (barobo_rpc_Request_Type_FIRE == rp.request.type) {
                    if (!rp.request.has_fire) {
                        rc_ = Status::PROTOCOL_ERROR;
                        yield break;
                    }
                    else {
                        yield {
                            Status status;
                            auto reply = serve(rp.request.fire, status);
                            if (hasError(status)) {
                                rc_ = status;
                                break;
                            }
                            else {
                                server_.asyncSendReply(rp.id, reply, std::move(op));
                            }
                        }
                    }
                }
                else {
                    rc_ = Status::PROTOCOL_ERROR;
                    yield break;
                }
            }
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
        }
    }

    barobo_rpc_Reply serve (barobo_rpc_Request_Fire fire, Status& status) {
        MethodInUnion<Interface> m;
        barobo_rpc_Reply reply = decltype(reply)();
        m.invoke(impl_, fire.id, fire.payload, reply.result.payload, status);
        if (!hasError(status)) {
            reply.type = barobo_rpc_Reply_Type_RESULT;
            reply.has_result = true;
            reply.result.id = fire.id;
        }
        return reply;
    }
};

template <class Interface, class S, class Impl, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, typename S::RequestHandlerSignature)
asyncServeUntilDisconnection (S& server, Impl& impl, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, typename S::RequestHandlerSignature
    > init { std::forward<CompletionToken>(token) };

    using Op = ServeUntilDisconnectionOperation<Interface, S, Impl>;
    util::asio::v1::makeOperation<Op>(std::move(init.handler), server, impl)();

    return init.result.get();
}

template <class Interface, class S, class Impl>
struct RunServerOperation {
    using RequestPair = typename S::RequestPair;

    RunServerOperation (S& server, Impl& impl) : server_(server), impl_(impl) {}

    S& server_;
    Impl& impl_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    auto result () {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}, RequestPair rp = {}) {
        if (!ec) reenter (op) {
            //yield asyncWaitForConnection(server_, std::move(op));
            //BOOST_LOG(server_.log()) << "connection received";
            //yield asyncReply(server_, rp.id, Versions::create<Interface>(), std::move(op));
            BOOST_LOG(server_.log()) << "now serving!";
            yield asyncServeUntilDisconnection<Interface>(server_, impl_, std::move(op));
            BOOST_LOG(server_.log()) << "finished serving";
            yield asyncReply(server_, rp.id, Status::OK, std::move(op));
            rc_ = ec;
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
        }
    }
};

template <class Interface, class S, class Impl, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::system::error_code))
asyncRunServer (S& server, Impl& impl, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, void(boost::system::error_code)
    > init { std::forward<CompletionToken>(token) };

    using Op = RunServerOperation<Interface, S, Impl>;
    util::asio::v1::makeOperation<Op>(std::move(init.handler), server, impl)();

    return init.result.get();
}

} // namespace asio
} // namespace rpc

#include <boost/asio/unyield.hpp>

#endif
