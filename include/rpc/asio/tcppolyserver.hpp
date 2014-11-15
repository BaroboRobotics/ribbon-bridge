#ifndef RPC_ASIO_TCPPOLYSERVER_HPP
#define RPC_ASIO_TCPPOLYSERVER_HPP

#include "rpc/asio/server.hpp"

#include "sfp/asio/messagequeue.hpp"

#include <boost/optional.hpp>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include <map>
#include <memory>
#include <thread>
#include <utility>

namespace rpc {
namespace asio {

using IoService = boost::asio::io_service;
namespace detail {

// Create an object of this type, then copy it as much as you want, passing it
// as a handler to multiple asynchronous operations producing an error_code.
// When the last copy is destroyed, the original handler is posted with the
// last erroneous error_code received, or success if no errors were reported.
template <class Handler>
struct WaitMultipleCompleter {
    WaitMultipleCompleter (IoService& ios, Handler handler)
        : mIos(ios)
        , mHandler(std::make_shared<Handler>(handler))
        , mErrorCode(std::make_shared<boost::system::error_code>())
    {}

    ~WaitMultipleCompleter () {
        if (mHandler.unique()) {
            mIos.post(std::bind(*mHandler, *mErrorCode));
        }
    }

    void operator() (boost::system::error_code ec) {
        if (ec) {
            if (*mErrorCode) {
                BOOST_LOG(mLog) << "multiple errors reported to a WaitMultipleCompleter, discarding "
                                << '"' << mErrorCode->message() << '"';
            }
            *mErrorCode = ec;
        }
    }

    mutable boost::log::sources::logger mLog;

    IoService& mIos;
    std::shared_ptr<Handler> mHandler;
    std::shared_ptr<boost::system::error_code> mErrorCode;
};

} // namespace detail

using Tcp = boost::asio::ip::tcp;

class TcpPolyServerImpl : public std::enable_shared_from_this<TcpPolyServerImpl> {
public:
    using SubServer = Server<sfp::asio::MessageQueue<Tcp::socket>>;

    using RequestId = std::pair<Tcp::endpoint, typename SubServer::RequestId>;
    using RequestPair = std::pair<RequestId, barobo_rpc_Request>;

    using RequestHandlerSignature = void(boost::system::error_code, RequestPair);
    using RequestHandler = std::function<RequestHandlerSignature>;

    using ReplyHandlerSignature = void(boost::system::error_code);
    using ReplyHandler = std::function<ReplyHandlerSignature>;

    using BroadcastHandlerSignature = void(boost::system::error_code);
    using BroadcastHandler = std::function<BroadcastHandlerSignature>;

    TcpPolyServerImpl (IoService& ioService)
        : mStrand(ioService)
        , mAcceptor(ioService)
    {}

    void init (Tcp::endpoint endpoint) {
        // Replicate what the Tcp::acceptor(ioService, endpoint) ctor would do.
        mAcceptor.open(endpoint.protocol());
        mAcceptor.set_option(boost::asio::socket_base::reuse_address(true));
        mAcceptor.bind(endpoint);
        mAcceptor.listen();

        boost::asio::spawn(mStrand,
            std::bind(&TcpPolyServerImpl::acceptorCoroutine,
                this->shared_from_this(), _1));
    }

    void postReceives () {
        BOOST_LOG(mLog) << "inbox: " << mInbox.size() << " -- receives: " << mReceives.size();
        while (mInbox.size() && mReceives.size()) {
            auto& ios = mReceives.front().first.get_io_service();
            ios.post(std::bind(mReceives.front().second, boost::system::error_code(), mInbox.front()));
            mInbox.pop();
            mReceives.pop();
        }
    }

    void asyncReceiveRequestImpl (IoService::work work, RequestHandler handler) {
        mReceives.emplace(std::piecewise_construct,
            std::forward_as_tuple(work), std::forward_as_tuple(handler));
        postReceives();
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        RequestHandlerSignature)
    asyncReceiveRequest (IoService::work work, Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, RequestHandlerSignature
        > init { std::forward<Handler>(handler) };

        mStrand.post(std::bind(&TcpPolyServerImpl::asyncReceiveRequestImpl,
            this->shared_from_this(), work, init.handler));

        return init.result.get();
    }

    void asyncSendReplyImpl (IoService::work work, RequestId requestId, barobo_rpc_Reply reply, ReplyHandler handler) {
        auto iter = mSubServers.find(requestId.first);
        if (iter != mSubServers.end()) {
            iter->second.asyncSendReply(requestId.second, reply,
                [work, handler] (boost::system::error_code ec) mutable {
                    auto& ios = work.get_io_service();
                    ios.post(std::bind(handler, ec));
                });
        }
        else if (requestId.first == Tcp::endpoint()) {
            BOOST_LOG(mLog) << "Reply to inaddr_any endpoint in polyserver, probably to our DISCONNECT, ignoring";
            auto& ios = work.get_io_service();
            ios.post(std::bind(handler, boost::system::error_code()));
        }
        else {
            auto& ios = work.get_io_service();
            ios.post(std::bind(handler, Status::NOT_CONNECTED));
        }
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        ReplyHandlerSignature)
    asyncSendReply (IoService::work work, RequestId requestId, barobo_rpc_Reply reply, Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, ReplyHandlerSignature
        > init { std::forward<Handler>(handler) };

        mStrand.post(std::bind(&TcpPolyServerImpl::asyncSendReplyImpl,
            this->shared_from_this(), work, requestId, reply, init.handler));

        return init.result.get();
    }

    void asyncSendBroadcastImpl (IoService::work work, barobo_rpc_Broadcast broadcast, BroadcastHandler handler) {
        auto& ios = work.get_io_service();
        detail::WaitMultipleCompleter<BroadcastHandler> completer { ios, handler };
        for (auto& kv : mSubServers) {
            BOOST_LOG(mLog) << "Broadcasting to " << kv.first;
            kv.second.asyncSendBroadcast(broadcast, completer);
        }
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        BroadcastHandlerSignature)
    asyncSendBroadcast (IoService::work work, barobo_rpc_Broadcast broadcast, Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, BroadcastHandlerSignature
        > init { std::forward<Handler>(handler) };

        mStrand.post(std::bind(&TcpPolyServerImpl::asyncSendBroadcastImpl,
            this->shared_from_this(), work, broadcast, init.handler));

        return init.result.get();
    }

    void subServerCoroutine (Tcp::endpoint peer, boost::asio::yield_context yield) {
        try {
            auto& server = mSubServers.at(peer);
            server.messageQueue().asyncHandshake(yield);

            SubServer::RequestId requestId;
            barobo_rpc_Request request;
            // Loop through incoming requests until we see a CONNECT. All other
            // requests get denied with a NOT_CONNECTED status.
            std::tie(requestId, request) = server.asyncReceiveRequest(yield);
            while (barobo_rpc_Request_Type_CONNECT != request.type) {
                BOOST_LOG(mLog) << "Ignoring non-CONNECT packet from " << peer;
                asyncReply(server, requestId, Status::NOT_CONNECTED, yield);
                std::tie(requestId, request) = server.asyncReceiveRequest(yield);
            }

            // We received a connection request, forward it up.
            mInbox.push(std::make_pair(std::make_pair(peer, requestId), request));
            postReceives();

            std::tie(requestId, request) = server.asyncReceiveRequest(yield);
            while (barobo_rpc_Request_Type_DISCONNECT != request.type) {
                mInbox.push(std::make_pair(std::make_pair(peer, requestId), request));
                postReceives();
                std::tie(requestId, request) = server.asyncReceiveRequest(yield);
            }

            asyncReply(server, requestId, Status::OK, yield);

            server.messageQueue().asyncShutdown(yield);
            server.messageQueue().stream().close();
        }
        catch (boost::system::system_error) {
            BOOST_LOG(mLog) << "Error communicating with " << peer;
        }

        BOOST_LOG(mLog) << peer << " disconnected";

        mSubServers.erase(peer);
        if (!mSubServers.size() && mShutdownOnLastDisconnect) {
            BOOST_LOG(mLog) << "Shutting down acceptor coroutine";
            mAcceptor.cancel();

            BOOST_LOG(mLog) << "Emitting disconnect";
            barobo_rpc_Request request;
            request = decltype(request)();
            request.type = barobo_rpc_Request_Type_DISCONNECT;
            mInbox.push(std::make_pair(std::make_pair(Tcp::endpoint(), nextRequestId()), request));
            postReceives();
        }
    }

    void acceptorCoroutine (boost::asio::yield_context yield) {
        try {
            do {
                Tcp::socket socket { mStrand.get_io_service() };
                Tcp::endpoint peer;
                mAcceptor.async_accept(socket, peer, yield);

                decltype(mSubServers)::iterator iter;
                bool success;
                std::tie(iter, success) = mSubServers.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(peer),
                    std::forward_as_tuple(std::move(socket)));
                assert(success);

                boost::asio::spawn(yield,
                    std::bind(&TcpPolyServerImpl::subServerCoroutine,
                        this->shared_from_this(), peer, _1));
            } while (mSubServers.size());
        }
        catch (boost::system::system_error& e) {
            BOOST_LOG(mLog) << "Error accepting new connection: " << e.what();
        }
    }

private:

    SubServer::RequestId nextRequestId () {
        return mNextRequestId++;
    }

    mutable boost::log::sources::logger mLog;

    bool mShutdownOnLastDisconnect = true;

    IoService::strand mStrand;
    Tcp::acceptor mAcceptor;

    SubServer::RequestId mNextRequestId = 0;

    std::map<Tcp::endpoint, SubServer> mSubServers;

    std::queue<std::pair<RequestId, barobo_rpc_Request>> mInbox;
    std::queue<std::pair<IoService::work, RequestHandler>> mReceives;
};

template <class Impl = TcpPolyServerImpl>
class TcpPolyServerService : public IoService::service {
public:
    static IoService::id id;

    using RequestId = typename Impl::RequestId;
    using RequestPair = typename Impl::RequestPair;

    explicit TcpPolyServerService (IoService& ioService)
        : IoService::service(ioService)
        , mAsyncWork(boost::in_place(std::ref(mAsyncIoService)))
        , mAsyncThread(static_cast<size_t(IoService::*)()>(&IoService::run), &mAsyncIoService)
    {}

    ~TcpPolyServerService () {
        mAsyncWork = boost::none;
        mAsyncIoService.stop();
        mAsyncThread.join();
    }

    using implementation_type = std::shared_ptr<Impl>;

    void construct (implementation_type& impl) {
        impl.reset(new Impl(mAsyncIoService));
    }

    void destroy (implementation_type& impl) {
        impl.reset();
    }

    void init (implementation_type& impl, Tcp::endpoint endpoint) {
        impl->init(endpoint);
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        void(boost::system::error_code, RequestPair))
    asyncReceiveRequest (implementation_type& impl, Handler&& handler) {
        IoService::work work { this->get_io_service() };
        return impl->asyncReceiveRequest(work, std::forward<Handler>(handler));
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        void(boost::system::error_code))
    asyncSendReply (implementation_type& impl,
            RequestId requestId, barobo_rpc_Reply reply, Handler&& handler) {
        IoService::work work { this->get_io_service() };
        return impl->asyncSendReply(work, requestId, reply, std::forward<Handler>(handler));
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        void(boost::system::error_code))
    asyncSendBroadcast (implementation_type& impl,
            barobo_rpc_Broadcast broadcast, Handler&& handler) {
        IoService::work work { this->get_io_service() };
        return impl->asyncSendBroadcast(work, broadcast, std::forward<Handler>(handler));
    }

private:
    void shutdown_service () {}

    IoService mAsyncIoService;
    boost::optional<IoService::work> mAsyncWork;
    std::thread mAsyncThread;
};

template <class Impl>
boost::asio::io_service::id TcpPolyServerService<Impl>::id;

template <class Service = TcpPolyServerService<>>
class BasicTcpPolyServer : public boost::asio::basic_io_object<Service> {
public:
    using RequestId = typename Service::RequestId;
    using RequestPair = typename Service::RequestPair;

    BasicTcpPolyServer (IoService& ioService, Tcp::endpoint endpoint)
        : boost::asio::basic_io_object<Service>(ioService)
    {
        this->get_service().init(this->get_implementation(), endpoint);
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        void(boost::system::error_code, RequestPair))
    asyncReceiveRequest (Handler&& handler) {
        return this->get_service().asyncReceiveRequest(this->get_implementation(),
            std::forward<Handler>(handler));
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        void(boost::system::error_code))
    asyncSendReply (RequestId requestId, barobo_rpc_Reply reply, Handler&& handler) {
        return this->get_service().asyncSendReply(this->get_implementation(),
            requestId, reply, std::forward<Handler>(handler));
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        void(boost::system::error_code))
    asyncSendBroadcast (barobo_rpc_Broadcast broadcast, Handler&& handler) {
        return this->get_service().asyncSendBroadcast(this->get_implementation(),
            broadcast, std::forward<Handler>(handler));
    }
};

using TcpPolyServer = BasicTcpPolyServer<>;

} // namespace asio
} // namespace rpc

#endif
