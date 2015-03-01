#ifndef RPC_ASIO_SERVER_HPP
#define RPC_ASIO_SERVER_HPP

#include "rpc.pb.h"

#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <functional>
#include <utility>

namespace rpc {
namespace asio {

template <class MessageQueue>
class Server {
public:
    using RequestId = uint32_t;
    using RequestPair = std::pair<RequestId, barobo_rpc_Request>;

    using RequestHandlerSignature = void(boost::system::error_code, RequestPair);
    using RequestHandler = std::function<RequestHandlerSignature>;

	explicit Server (boost::asio::io_service& ios, boost::log::sources::logger log)
		: mMessageQueue(ios, log)
        , mLog(log)
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

    boost::log::sources::logger log () { return mLog; }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, RequestHandlerSignature)
    asyncReceiveRequest (Handler&& handler) {
        boost::asio::detail::async_result_init<
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
                            std::bind(realHandler, status, std::make_pair(message.id, message.request)));
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
        boost::asio::detail::async_result_init<
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
            size_t bytesWritten;
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
        boost::asio::detail::async_result_init<
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
            size_t bytesWritten;
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

    boost::log::sources::logger mLog;
};

template <class S, class Broadcast, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncBroadcast (S& server, Broadcast args, Handler&& handler) {
    boost::asio::detail::async_result_init<
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
asyncReply (S& server, typename S::RequestId requestId, ServiceInfo info, Handler&& handler) {
    barobo_rpc_Reply reply;
    reply = decltype(reply)();
    reply.type = barobo_rpc_Reply_Type_SERVICEINFO;
    reply.has_serviceInfo = true;
    reply.serviceInfo = info;
    server.asyncSendReply(requestId, reply, std::forward<Handler>(handler));
}

template <class S, class Handler>
struct WaitForConnectionOperation : std::enable_shared_from_this<WaitForConnectionOperation<S, Handler>> {
    using RequestPair = typename S::RequestPair;

    WaitForConnectionOperation (S& server)
        : mIos(server.get_io_service())
        , mStrand(mIos)
        , mServer(server)
    {}

    void start (Handler handler) {
        mServer.asyncReceiveRequest(mStrand.wrap(
            std::bind(&WaitForConnectionOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (Handler handler, boost::system::error_code ec, RequestPair rp) {
        if (!ec) {
            auto& requestId = rp.first;
            auto& request = rp.second;
            if (barobo_rpc_Request_Type_CONNECT != request.type) {
                asyncReply(mServer, requestId, rpc::Status::NOT_CONNECTED, mStrand.wrap(
                    std::bind(&WaitForConnectionOperation::stepTwo,
                        this->shared_from_this(), handler, _1)));
            }
            else {
                mIos.post(std::bind(handler, ec, rp));
            }
        }
        else {
            mIos.post(std::bind(handler, ec, rp));
        }
    }

    void stepTwo (Handler handler, boost::system::error_code ec) {
        if (!ec) {
            start(handler);
        }
        else {
            mIos.post(std::bind(handler, ec, RequestPair()));
        }
    }

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    S& mServer;
};

template <class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, typename S::RequestHandlerSignature)
asyncWaitForConnection (S& server, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, typename S::RequestHandlerSignature
    > init { std::forward<Handler>(handler) };

    using Op = WaitForConnectionOperation<S, decltype(init.handler)>;
    std::make_shared<Op>(server)->start(init.handler);

    return init.result.get();
}

template <class Interface, class S, class Impl, class Handler>
struct ServeUntilDisconnectionOperation : std::enable_shared_from_this<ServeUntilDisconnectionOperation<Interface, S, Impl, Handler>> {
    using RequestPair = typename S::RequestPair;

    ServeUntilDisconnectionOperation (S& server, Impl& impl)
        : mIos(server.get_io_service())
        , mStrand(mIos)
        , mServer(server)
        , mImpl(impl)
    {}

    barobo_rpc_Reply serve (barobo_rpc_Request_Fire fire, Status& status) {
        MethodInUnion<Interface> m;
        barobo_rpc_Reply reply = decltype(reply)();
        m.invoke(mImpl, fire.id, fire.payload, reply.result.payload, status);
        if (!hasError(status)) {
            reply.type = barobo_rpc_Reply_Type_RESULT;
            reply.has_result = true;
            reply.result.id = fire.id;
        }
        return reply;
    }

    void start (Handler handler) {
        mServer.asyncReceiveRequest(mStrand.wrap(
            std::bind(&ServeUntilDisconnectionOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (Handler handler, boost::system::error_code ec, RequestPair rp) {
        if (!ec) {
            auto& requestId = rp.first;
            auto& request = rp.second;

            if (barobo_rpc_Request_Type_DISCONNECT == request.type) {
                mIos.post(std::bind(handler, ec, rp));
            }
            else if (barobo_rpc_Request_Type_CONNECT == request.type) {
                asyncReply(mServer, requestId, ServiceInfo::create<Interface>(), mStrand.wrap(
                    std::bind(&ServeUntilDisconnectionOperation::stepTwo,
                        this->shared_from_this(), handler, _1)));
            }
            else if (barobo_rpc_Request_Type_FIRE == request.type) {
                if (!request.has_fire) {
                    mIos.post(std::bind(handler, Status::INCONSISTENT_REPLY, rp));
                }
                else {
                    Status status;
                    auto reply = serve(request.fire, status);
                    if (hasError(status)) {
                        mIos.post(std::bind(handler, status, rp));
                    }
                    else {
                        mServer.asyncSendReply(requestId, reply, mStrand.wrap(
                            std::bind(&ServeUntilDisconnectionOperation::stepTwo,
                                this->shared_from_this(), handler, _1)));
                    }
                }
            }
            else {
                mIos.post(std::bind(handler, Status::INCONSISTENT_REPLY, rp));
            }
        }
        else {
            mIos.post(std::bind(handler, ec, rp));
        }
    }

    void stepTwo (Handler handler, boost::system::error_code ec) {
        if (!ec) {
            start(handler);
        }
        else {
            mIos.post(std::bind(handler, ec, RequestPair()));
        }
    }

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    S& mServer;
    Impl& mImpl;
};

template <class Interface, class S, class Impl, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, typename S::RequestHandlerSignature)
asyncServeUntilDisconnection (S& server, Impl& impl, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, typename S::RequestHandlerSignature
    > init { std::forward<Handler>(handler) };

    using Op = ServeUntilDisconnectionOperation<Interface, S, Impl, decltype(init.handler)>;
    std::make_shared<Op>(server, impl)->start(init.handler);

    return init.result.get();
}

template <class Interface, class S, class Impl, class Handler>
struct RunServerOperation : std::enable_shared_from_this<RunServerOperation<Interface, S, Impl, Handler>> {
    using RequestPair = typename S::RequestPair;

    RunServerOperation (S& server, Impl& impl)
        : mIos(server.get_io_service())
        , mStrand(mIos)
        , mServer(server)
        , mImpl(impl)
    {}

    void start (Handler handler) {
        asyncWaitForConnection(mServer, mStrand.wrap(
            std::bind(&RunServerOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (Handler handler, boost::system::error_code ec, RequestPair rp) {
        if (!ec) {
            auto log = mServer.log();
            BOOST_LOG(log) << "connection received";
            auto& requestId = rp.first;
            asyncReply(mServer, requestId, ServiceInfo::create<Interface>(), mStrand.wrap(
                std::bind(&RunServerOperation::stepTwo,
                    this->shared_from_this(), handler, _1)));
        }
        else {
            mIos.post(std::bind(handler, ec));
        }
    }

    void stepTwo (Handler handler, boost::system::error_code ec) {
        if (!ec) {
            auto log = mServer.log();
            BOOST_LOG(log) << "now serving!";
            asyncServeUntilDisconnection<Interface>(mServer, mImpl, mStrand.wrap(
                std::bind(&RunServerOperation::stepThree,
                    this->shared_from_this(), handler, _1, _2)));
        }
        else {
            mIos.post(std::bind(handler, ec));
        }
    }

    void stepThree (Handler handler, boost::system::error_code ec, RequestPair rp) {
        if (!ec) {
            auto log = mServer.log();
            BOOST_LOG(log) << "finished serving";
            auto& requestId = rp.first;
            asyncReply(mServer, requestId, Status::OK, [handler, log] (boost::system::error_code ec2) mutable {
                handler(ec2);
            });
        }
        else {
            mIos.post(std::bind(handler, ec));
        }
    }

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    S& mServer;
    Impl& mImpl;
};

template <class Interface, class S, class Impl, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncRunServer (S& server, Impl& impl, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };

    using Op = RunServerOperation<Interface, S, Impl, decltype(init.handler)>;
    std::make_shared<Op>(server, impl)->start(init.handler);

    return init.result.get();
}

} // namespace asio
} // namespace rpc

#endif
