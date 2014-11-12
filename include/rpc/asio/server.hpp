#ifndef RPC_ASIO_SERVER_HPP
#define RPC_ASIO_SERVER_HPP

#include "util/hexdump.hpp"

#include <utility>

namespace rpc {
namespace asio {

template <class MessageQueue>
class Server {
public:
    using RequestId = uint32_t;

	template <class... Args>
	explicit Server (Args&&... args)
		: mMessageQueue(std::forward<Args>(args)...)
	{}

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
                mMessageQueue.getIoService().post(
                    std::bind(realHandler, status, std::make_pair(message.id, message.request)));
            });

        return init.result.get();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
    asyncReply (RequestId requestId, barobo_rpc_Reply reply, Handler&& handler) {
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
            mMessageQueue.getIoService().post(std::bind(realHandler, e.code()));
        }

        return init.result.get();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
    asyncBroadcast (barobo_rpc_Broadcast broadcast, Handler&& handler) {
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
            mMessageQueue.getIoService().post(std::bind(realHandler, e.code()));
        }

        return init.result.get();
    }

private:
    mutable boost::log::sources::logger mLog;

	MessageQueue mMessageQueue;
};

} // namespace asio
} // namespace rpc

#endif