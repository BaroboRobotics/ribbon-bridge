#ifndef RPC_ASIO_CLIENT_HPP
#define RPC_ASIO_CLIENT_HPP

#include "rpc/system_error.hpp"
#include "rpc/version.hpp"

#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>

#include <boost/unordered_map.hpp>

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include <memory>
#include <utility>

namespace rpc {
namespace asio {

using namespace std::placeholders;

template <class MessageQueue>
class Client {
public:
    template <class... Args>
    explicit Client (Args&&... args)
        : mImpl(std::make_shared<Impl>(std::forward<Args>(args)...))
    {}

    MessageQueue& messageQueue () { return mImpl->mMessageQueue; }
    const MessageQueue& messageQueue () const { return mImpl->mMessageQueue; }

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
            size_t bytesWritten;
            rpc::encode(message, buf->data(), buf->size(), bytesWritten);
            buf->resize(bytesWritten);

            m->mStrand.post([m, buf, requestId, realHandler, timeout] () {
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
            m->mMessageQueue.getIoService().post(std::bind(realHandler, e.code(), barobo_rpc_Reply()));
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
    struct Impl : std::enable_shared_from_this<Impl> {
        template <class... Args>
        explicit Impl (Args&&... args)
            : mMessageQueue(std::forward<Args>(args)...)
            , mIoService(mMessageQueue.getIoService())
            , mStrand(mIoService)
        {}

        void handleReply (uint32_t requestId, boost::system::error_code ec, barobo_rpc_Reply reply) {
            auto handlerIter = mReplyHandlers.find(requestId);
            if (mReplyHandlers.cend() != handlerIter) {
                mIoService.post(std::bind(handlerIter->second, ec, reply));
                mReplyHandlers.erase(handlerIter);
            }
            else {
                BOOST_LOG(mLog) << "client: Unsolicited reply to request " << requestId;
            }

            auto timeoutIter = mReplyTimeouts.find(requestId);
            if (mReplyTimeouts.cend() != timeoutIter) {
                timeoutIter->second.cancel();
                mReplyTimeouts.erase(timeoutIter);
            }
        }

        void emplaceReplyHandler (uint32_t key, ReplyHandler replyHandler) {
            mReplyHandlers.emplace(std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(replyHandler));
            startReceiveCoroutine();
        }

        template <class Duration>
        void emplaceReplyTimeout (uint32_t requestId, Duration&& timeout) {
            auto& timer = mReplyTimeouts.emplace(std::piecewise_construct,
                std::forward_as_tuple(requestId),
                std::forward_as_tuple(mIoService)).first->second;
            timer.expires_from_now(timeout);
            auto m = this->shared_from_this();
            timer.async_wait(mStrand.wrap([m, requestId] (boost::system::error_code ec) {
                if (!ec) {
                    m->handleReply(requestId, boost::asio::error::timed_out, barobo_rpc_Reply());
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
            if (mReceiveCoroutineRunning) {
                return;
            }
            mReceiveCoroutineRunning = true;
            boost::asio::spawn(mStrand, std::bind(&Impl::receiveCoroutine, this->shared_from_this(), _1));
        }

        void receiveCoroutine (boost::asio::yield_context yield) {
            try {
                std::vector<uint8_t> buf(1024);
                while (mReplyHandlers.size() || mBroadcastHandlers.size()) {
                    auto size = mMessageQueue.asyncReceive(boost::asio::buffer(buf), yield);
                    barobo_rpc_ServerMessage message;
                    decode(message, buf.data(), size);

                    switch (message.type) {
                        case barobo_rpc_ServerMessage_Type_REPLY:
                            if (!message.has_inReplyTo) {
                                // FIXME INCONSISTENT_REPLY should be INCONSISTENT_MESSAGE
                                throw Error(Status::INCONSISTENT_REPLY);
                            }
                            mReplyInbox.push(std::make_pair(message.inReplyTo, message.reply));
                            break;
                        case barobo_rpc_ServerMessage_Type_BROADCAST:
                            if (message.has_inReplyTo) {
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
            }
            catch (boost::system::system_error& e) {
                voidHandlers(e.code());
            }
            mReceiveCoroutineRunning = false;
        }

        void voidHandlers (boost::system::error_code ec) {
            BOOST_LOG(mLog) << "client: voiding all handlers with " << ec.message();
            for (auto& pair : mReplyHandlers) {
                BOOST_LOG(mLog) << "client: voiding request " << pair.first << " with " << ec.message();
                mIoService.post(std::bind(pair.second, ec, barobo_rpc_Reply()));
            }
            mReplyHandlers.clear();
            for (auto& pair: mReplyTimeouts) {
                pair.second.cancel();
            }
            mReplyTimeouts.clear();
            while (mBroadcastHandlers.size()) {
                BOOST_LOG(mLog) << "client: voiding broadcast handler with " << ec.message();
                mIoService.post(std::bind(mBroadcastHandlers.front(), ec, barobo_rpc_Broadcast()));
                mBroadcastHandlers.pop();
            }
        }

        uint32_t nextRequestId () { return mNextRequestId++; }

        mutable boost::log::sources::logger mLog;

        MessageQueue mMessageQueue;
        boost::asio::io_service& mIoService;
        boost::asio::io_service::strand mStrand;

        bool mReceiveCoroutineRunning = false;

        std::atomic<uint32_t> mNextRequestId = { 0 };

        std::queue<std::pair<uint32_t, barobo_rpc_Reply>> mReplyInbox;
        boost::unordered_map<uint32_t, ReplyHandler> mReplyHandlers;
        boost::unordered_map<uint32_t, boost::asio::steady_timer> mReplyTimeouts;

        std::queue<barobo_rpc_Broadcast> mBroadcastInbox;
        std::queue<BroadcastHandler> mBroadcastHandlers;
    };


    std::shared_ptr<Impl> mImpl;
};

} // namespace asio
} // namespace rpc

#endif