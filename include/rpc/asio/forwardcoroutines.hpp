#ifndef RPC_ASIO_FORWARDCOROUTINES_HPP
#define RPC_ASIO_FORWARDCOROUTINES_HPP

#include "rpc.pb.h"

#include "rpc/asio/waitmultiplecompleter.hpp"

#include <boost/asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include <chrono>

namespace rpc {
namespace asio {

using ForwardRequestsHandlerSignature = void(boost::system::error_code);
using ForwardRequestsHandler = std::function<ForwardRequestsHandlerSignature>;

using ForwardBroadcastsHandlerSignature = void(boost::system::error_code);
using ForwardBroadcastsHandler = std::function<ForwardBroadcastsHandlerSignature>;

using RunProxyHandlerSignature = void(boost::system::error_code);
using RunProxyHandler = std::function<RunProxyHandlerSignature>;

template <class C, class S, class Handler>
struct ForwardRequestsOperation : std::enable_shared_from_this<ForwardRequestsOperation<C, S, Handler>> {
    using RequestId = typename S::RequestId;
    using RequestPair = typename S::RequestPair;

    using MultiHandler = WaitMultipleCompleter<Handler>;

    ForwardRequestsOperation (C& client, S& server)
        : mIos(server.get_io_service())
        , mStrand(mIos)
        , mClient(client)
        , mServer(server)
    {}

    void start (Handler handler) {
        startImpl(WaitMultipleCompleter<Handler>(mIos, handler));
    }

    void startImpl (MultiHandler handler) {
        mServer.asyncReceiveRequest(mStrand.wrap(
            std::bind(&ForwardRequestsOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (MultiHandler handler, boost::system::error_code ec, RequestPair rp) {
        auto log = mServer.log();
        if (!ec) {
            auto& requestId = rp.first;
            auto& request = rp.second;
            mClient.asyncRequest(request, std::chrono::seconds(60), mStrand.wrap(
                std::bind(&ForwardRequestsOperation::stepTwo,
                    this->shared_from_this(), handler, requestId, _1, _2)));
            startImpl(handler);
        }
        else {
            BOOST_LOG(log) << "ForwardRequestsOperation: Error receiving request: " << ec.message();
            mIos.post(std::bind(handler, ec));
            mClient.close();
        }
    }

    void stepTwo (MultiHandler handler, RequestId requestId, boost::system::error_code ec, barobo_rpc_Reply reply) {
        auto log = mServer.log();
        if (!ec) {
            mServer.asyncSendReply(requestId, reply, mStrand.wrap(
                std::bind(&ForwardRequestsOperation::stepThree,
                    this->shared_from_this(), handler, _1)));
        }
        else {
            BOOST_LOG(log) << "ForwardRequestsOperation: Error forwarding request: " << ec.message();
            mIos.post(std::bind(handler, ec));
            mServer.close();
        }
    }

    void stepThree (MultiHandler handler, boost::system::error_code ec) {
        auto log = mServer.log();
        mIos.post(std::bind(handler, ec));
        if (ec) {
            BOOST_LOG(log) << "ForwardRequestsOperation: Error replying to request: " << ec.message();
            mClient.close();
        }
    }

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    C& mClient;
    S& mServer;
    ForwardRequestsHandler mHandler;
};

template <class C, class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, ForwardRequestsHandlerSignature)
asyncForwardRequests (C& client, S& server, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, ForwardRequestsHandlerSignature
    > init { std::forward<Handler>(handler) };

    using Op = ForwardRequestsOperation<C, S, decltype(init.handler)>;
    std::make_shared<Op>(client, server)->start(init.handler);

    return init.result.get();
}

template <class C, class S, class Handler>
struct ForwardBroadcastsOperation : std::enable_shared_from_this<ForwardBroadcastsOperation<C, S, Handler>> {
    ForwardBroadcastsOperation (C& client, S& server)
        : mIos(client.get_io_service())
        , mStrand(mIos)
        , mClient(client)
        , mServer(server)
    {}

    void start (Handler handler) {
        mClient.asyncReceiveBroadcast(mStrand.wrap(
            std::bind(&ForwardBroadcastsOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (Handler handler, boost::system::error_code ec, barobo_rpc_Broadcast broadcast) {
        if (!ec) {
            mServer.asyncSendBroadcast(broadcast, mStrand.wrap(
                std::bind(&ForwardBroadcastsOperation::stepTwo,
                    this->shared_from_this(), handler, _1)));
        }
        else {
            mIos.post(std::bind(handler, ec));
        }
    }

    void stepTwo (Handler handler, boost::system::error_code ec) {
        if (!ec) {
            start(handler);
        }
        else {
            mIos.post(std::bind(handler, ec));
        }
    }

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    C& mClient;
    S& mServer;
};

template <class C, class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, ForwardBroadcastsHandlerSignature)
asyncForwardBroadcasts (C& client, S& server, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, ForwardBroadcastsHandlerSignature
    > init { std::forward<Handler>(handler) };

    using Op = ForwardBroadcastsOperation<C, S, decltype(init.handler)>;
    std::make_shared<Op>(client, server)->start(init.handler);

    return init.result.get();
}

// Forward requests from the given server to the given client, forward replies
// and broadcasts from the client to the server. Stops only when there is an
// error reported by either the client or the server.
template <class C, class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, RunProxyHandlerSignature)
asyncRunProxy (C& client, S& server, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, ForwardBroadcastsHandlerSignature
    > init { std::forward<Handler>(handler) };

    WaitMultipleCompleter<decltype(init.handler)> multiHandler {
        server.get_io_service(), init.handler
    };
    asyncForwardRequests(client, server, multiHandler);
    asyncForwardBroadcasts(client, server, multiHandler);

    return init.result.get();
}

} // namespace asio
} // namespace rpc

#endif