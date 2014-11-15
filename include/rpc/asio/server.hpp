#ifndef RPC_ASIO_SERVER_HPP
#define RPC_ASIO_SERVER_HPP

#include "rpc.pb.h"

#include "util/hexdump.hpp"

#include <utility>

namespace rpc {
namespace asio {

template <class MessageQueue>
class Server {
public:
    using RequestId = uint32_t;
    using RequestPair = std::pair<RequestId, barobo_rpc_Request>;

	template <class... Args>
	explicit Server (Args&&... args)
		: mMessageQueue(std::forward<Args>(args)...)
	{}

    boost::asio::io_service& get_io_service () { return mMessageQueue.get_io_service(); }

	MessageQueue& messageQueue () { return mMessageQueue; }
	const MessageQueue& messageQueue () const { return mMessageQueue; }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code, std::pair<RequestId, barobo_rpc_Request>))
    asyncReceiveRequest (Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, void(boost::system::error_code, std::pair<RequestId, barobo_rpc_Request>)
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        auto buf = std::make_shared<std::vector<uint8_t>>(1024);
        mMessageQueue.asyncReceive(boost::asio::buffer(*buf),
            [this, realHandler, buf] (boost::system::error_code ec, size_t size) mutable {
                barobo_rpc_ClientMessage message;
                Status status;
                rpc::decode(message, buf->data(), size, status);
                BOOST_LOG(mLog) << "server received " << size << " bytes";
                mMessageQueue.get_io_service().post(
                    std::bind(realHandler, status, std::make_pair(message.id, message.request)));
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
            BOOST_LOG(mLog) << "server: posting error sending reply";
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
            BOOST_LOG(mLog) << "server: posting error sending broadcast";
            mMessageQueue.get_io_service().post(std::bind(realHandler, e.code()));
        }

        return init.result.get();
    }

private:
    mutable boost::log::sources::logger mLog;

	MessageQueue mMessageQueue;
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

template <class S, class ProcessorCoro>
typename S::RequestPair processRequestsCoro (S& server, ProcessorCoro&& process, boost::asio::yield_context yield) {
    auto rp = server.asyncReceiveRequest(yield);
    while (std::forward<ProcessorCoro>(process)(rp.first, rp.second, yield)) {
        rp = server.asyncReceiveRequest(yield);
    }
    return rp;
}

template <class S>
bool notConnectedCoro (S& server,
    typename S::RequestId requestId,
    barobo_rpc_Request request,
    boost::asio::yield_context yield)
{
    if (barobo_rpc_Request_Type_CONNECT != request.type) {
        asyncReply(server, requestId, rpc::Status::NOT_CONNECTED, yield);
        return true;
    }
    return false;
}


} // namespace asio
} // namespace rpc

#endif