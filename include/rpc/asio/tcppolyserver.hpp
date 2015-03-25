#ifndef RPC_ASIO_TCPPOLYSERVER_HPP
#define RPC_ASIO_TCPPOLYSERVER_HPP

#include "rpc/asio/tcpserver.hpp"
#include "rpc/asio/waitmultiplecompleter.hpp"

#include <boost/optional.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/basic_io_object.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/handler_invoke_hook.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace rpc {
namespace asio {

using IoService = boost::asio::io_service;
using namespace std::placeholders;

namespace _ {

// TODO refactor to combine this with RunServerOperation in server.hpp?
template <class S, class RequestFunc, class Handler>
struct RunSubServerOperation : std::enable_shared_from_this<RunSubServerOperation<S, RequestFunc, Handler>> {
    using RequestId = typename S::RequestId;
    using RequestPair = typename S::RequestPair;

    RunSubServerOperation (std::shared_ptr<S> subServer, RequestFunc requestFunc)
        : mIos(subServer->get_io_service())
        , mStrand(mIos)
        , mSubServer(subServer)
        , mRequestFunc(requestFunc)
    {}

    void start (Handler handler) {
        auto self = this->shared_from_this();
        mSubServer->messageQueue().asyncKeepalive(
            [self, this] (boost::system::error_code ec) {
                if (boost::asio::error::operation_aborted != ec) {
                    auto log = this->mSubServer->log();
                    BOOST_LOG(log) << "Subserver died with " << ec.message();
                    ec = boost::system::error_code{};
                    this->mSubServer->close(ec); // ignore error
                }
            });

        asyncWaitForConnection(*mSubServer, mStrand.wrap(
            std::bind(&RunSubServerOperation::stepOne,
                this->shared_from_this(), handler, _1, _2)));
    }

    void stepOne (Handler handler, boost::system::error_code ec, RequestPair rp) {
        auto log = mSubServer->log();
        if (!ec) {
            auto& requestId = rp.first;
            auto& request = rp.second;
            if (barobo_rpc_Request_Type_DISCONNECT == request.type) {
                BOOST_LOG(log) << "disconnecting";
                // Don't forward the request to mRequestFunc, as we may not be
                // the last subserver remaining. Our caller can figure out if
                // we are the last one and emit any last DISCONNECT request as
                // necessary.
                asyncReply(*mSubServer, requestId, Status::OK, handler);
            }
            else {
                if (barobo_rpc_Request_Type_CONNECT == request.type) {
                    BOOST_LOG(log) << "connection received";
                }
                using boost::asio::asio_handler_invoke;
                asio_handler_invoke(std::bind(mRequestFunc, requestId, request), &handler);
                // recurse
                mSubServer->asyncReceiveRequest(mStrand.wrap(
                    std::bind(&RunSubServerOperation::stepOne,
                        this->shared_from_this(), handler, _1, _2)));
            }
        }
        else {
            BOOST_LOG(log) << "subserver run interrupted: " << ec.message();
            mIos.post(std::bind(handler, ec));
        }
    }

    mutable boost::log::sources::logger mLog;

    boost::asio::io_service& mIos;
    boost::asio::io_service::strand mStrand;
    std::shared_ptr<S> mSubServer;
    RequestFunc mRequestFunc;
};

template <class S, class RequestFunc, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
asyncRunSubServer (std::shared_ptr<S> server, RequestFunc&& requestFunc, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, void(boost::system::error_code)
    > init { std::forward<Handler>(handler) };

    using Op = RunSubServerOperation<S, RequestFunc, decltype(init.handler)>;
    std::make_shared<Op>(server, std::forward<RequestFunc>(requestFunc))->start(init.handler);

    return init.result.get();
}

} // namespace _

class TcpPolyServerImpl : public std::enable_shared_from_this<TcpPolyServerImpl> {
public:
    using SubServer = TcpServer;

    using RequestId = std::pair<Tcp::endpoint, typename SubServer::RequestId>;
    using RequestPair = std::pair<RequestId, barobo_rpc_Request>;

    typedef void RequestHandlerSignature(boost::system::error_code, RequestPair);
    using RequestHandler = std::function<RequestHandlerSignature>;

    typedef void ReplyHandlerSignature(boost::system::error_code);
    using ReplyHandler = std::function<ReplyHandlerSignature>;

    typedef void BroadcastHandlerSignature(boost::system::error_code);
    using BroadcastHandler = std::function<BroadcastHandlerSignature>;

    explicit TcpPolyServerImpl (IoService& ioService)
        : mStrand(ioService)
        , mAcceptor(ioService)
    {}

    void destroy () {
        boost::system::error_code ec;
        close(ec);
    }

    void close (boost::system::error_code& ec) {
        mStrand.post(std::bind(&TcpPolyServerImpl::closeImpl,
            this->shared_from_this()));
        ec = boost::system::error_code{};
    }

    void init (Tcp::endpoint endpoint, boost::log::sources::logger log) {
        mLogPrototype = mLog = log;
        mLog.add_attribute("Protocol", boost::log::attributes::constant<std::string>("RB-PS"));

        mDesiredEndpoint = endpoint;
        initAcceptor();
    }

    Tcp::endpoint endpoint () const {
        // This may be different from mDesiredEndpoint, which may use service (port) "0".
        return mAcceptor.local_endpoint();
    }

    boost::log::sources::logger log () const {
        return mLog;
    }

    template <class Handler>
    BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
        RequestHandlerSignature)
    asyncReceiveRequest (IoService::work work, Handler&& handler) {
        boost::asio::detail::async_result_init<
            Handler, RequestHandlerSignature
        > init { std::forward<Handler>(handler) };
        auto& realHandler = init.handler;

        mStrand.post(std::bind(&TcpPolyServerImpl::asyncReceiveRequestImpl,
            this->shared_from_this(), work, realHandler));

        return init.result.get();
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

private:
    void initAcceptor () {
        if (mAcceptor.is_open()) {
            boost::system::error_code ec;
            mAcceptor.close(ec);
            if (ec) {
                BOOST_LOG(mLog) << "Error closing acceptor for reinitialization: " << ec.message();
            }
        }

        // Replicate what the Tcp::acceptor(ioService, endpoint) ctor would do.
        mAcceptor.open(mDesiredEndpoint.protocol());
        mAcceptor.set_option(boost::asio::socket_base::reuse_address(true));
        mAcceptor.bind(mDesiredEndpoint);
        mAcceptor.listen();

        mStrand.post(std::bind(&TcpPolyServerImpl::accept, this->shared_from_this()));
    }

    void accept () {
        auto subServer = std::make_shared<SubServer>(mStrand.get_io_service(), mLogPrototype);
        auto peer = std::make_shared<Tcp::endpoint>();
        mAcceptor.async_accept(subServer->messageQueue().stream(), *peer, mStrand.wrap(
            std::bind(&TcpPolyServerImpl::handleAccept,
                this->shared_from_this(), peer, subServer, _1)));
    }

    void handleAccept (std::shared_ptr<Tcp::endpoint> peer,
                       std::shared_ptr<SubServer> subServer,
                       boost::system::error_code ec) {
        if (!ec) {
            subServer->messageQueue().asyncHandshake(mStrand.wrap(
                std::bind(&TcpPolyServerImpl::handleHandshake,
                    this->shared_from_this(), peer, subServer, _1)));
            accept();
        }
        else {
            BOOST_LOG(mLog) << "Error accepting new connection: " << ec.message();
            if (boost::asio::error::operation_aborted != ec) {
                BOOST_LOG(mLog) << "Attempting to reinitialize acceptor";
                initAcceptor();
            }
        }
    }

    void handleHandshake (std::shared_ptr<Tcp::endpoint> peer,
                       std::shared_ptr<SubServer> subServer,
                       boost::system::error_code ec) {
        if (!ec) {
            decltype(mSubServers)::iterator iter;
            bool success;
            std::tie(iter, success) = mSubServers.insert(std::make_pair(*peer, subServer));
            assert(success);

            BOOST_LOG(mLog) << "inserted subserver for " << *peer;

            _::asyncRunSubServer(subServer,
                std::bind(&TcpPolyServerImpl::pushRequest, this->shared_from_this(), *peer, _1, _2),
                mStrand.wrap(std::bind(&TcpPolyServerImpl::handleSubServerFinished,
                    this->shared_from_this(), *peer, _1)));
        }
        else {
            BOOST_LOG(mLog) << "Handshake error with " << *peer << ": " << ec.message();
            if (boost::asio::error::operation_aborted != ec) {
                ec = boost::system::error_code{};
                subServer->close(ec); // ignore error
            }
        }
    }

    void closeImpl () {
        boost::system::error_code ec;
        mAcceptor.close(ec);
        if (ec) {
            BOOST_LOG(mLog) << "Error closing acceptor: " << ec.message();
        }
        for (auto& kv : mSubServers) {
            kv.second->close(ec);
            if (ec) {
                BOOST_LOG(mLog) << "Error closing subserver "
                                << kv.first << ": " << ec.message();
            }
        }
        voidReceives(boost::asio::error::operation_aborted);
    }

    void pushRequest (Tcp::endpoint peer, SubServer::RequestId requestId, barobo_rpc_Request request) {
        mInbox.push(std::make_pair(std::make_pair(peer, requestId), request));
        postReceives();
    }

    void handleSubServerFinished (Tcp::endpoint peer, boost::system::error_code ec) {
        BOOST_LOG(mLog) << "Subserver " << peer << " finished with " << ec.message();
        auto iter = mSubServers.find(peer);
        if (iter != mSubServers.end()) {
            ec = boost::system::error_code{};
            iter->second->close(ec); // ignore error
            mSubServers.erase(iter);
            BOOST_LOG(mLog) << peer << " erased; " << mSubServers.size() << " subservers remaining";
        }
        if (!mSubServers.size()) {
            BOOST_LOG(mLog) << "Emitting disconnect";
            barobo_rpc_Request request;
            request = decltype(request)();
            request.type = barobo_rpc_Request_Type_DISCONNECT;
            pushRequest(Tcp::endpoint(), nextRequestId(), request);
        }
    }

    void asyncReceiveRequestImpl (IoService::work work, RequestHandler handler) {
        mReceives.emplace(std::piecewise_construct,
            std::forward_as_tuple(work), std::forward_as_tuple(handler));
        postReceives();
    }

    void asyncSendReplyImpl (IoService::work work, RequestId requestId, barobo_rpc_Reply reply, ReplyHandler handler) {
        auto iter = mSubServers.find(requestId.first);
        if (iter != mSubServers.end()) {
            iter->second->asyncSendReply(requestId.second, reply,
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
            // If no subserver exists for this request ID, ignore this attempt
            // at a reply. We don't want to post an error, because the
            // situation is similar to a remote client ignoring spurious, or
            // expired, replies. The TcpPolyServer is still functional.
            auto& ios = work.get_io_service();
            ios.post(std::bind(handler, boost::system::error_code()));
        }
    }

    void asyncSendBroadcastImpl (IoService::work work, barobo_rpc_Broadcast broadcast, BroadcastHandler handler) {
        auto& ios = work.get_io_service();
        WaitMultipleCompleter<BroadcastHandler> completer { ios, handler };
        for (auto& kv : mSubServers) {
            BOOST_LOG(mLog) << "Broadcasting to " << kv.first;
            kv.second->asyncSendBroadcast(broadcast, completer);
        }
    }

    void postReceives () {
        while (mInbox.size() && mReceives.size()) {
            auto request = mInbox.front();
            auto op = mReceives.front();
            auto& ios = op.first.get_io_service();
            auto& handler = op.second;

            mInbox.pop();
            mReceives.pop();

            ios.post(std::bind(handler, boost::system::error_code(), request));
        }
    }

    void voidReceives (boost::system::error_code ec) {
        while (mReceives.size()) {
            auto op = mReceives.front();
            auto& ios = op.first.get_io_service();
            auto& handler = op.second;

            mReceives.pop();

            ios.post(std::bind(handler, ec, RequestPair()));
        }
    }

    SubServer::RequestId nextRequestId () {
        return mNextRequestId++;
    }

    IoService::strand mStrand;

    SubServer::RequestId mNextRequestId = 0;

    Tcp::endpoint mDesiredEndpoint;
    Tcp::acceptor mAcceptor;
    std::map<Tcp::endpoint, std::shared_ptr<SubServer>> mSubServers;

    std::queue<std::pair<RequestId, barobo_rpc_Request>> mInbox;
    std::queue<std::pair<IoService::work, RequestHandler>> mReceives;

    boost::log::sources::logger mLogPrototype;
    mutable boost::log::sources::logger mLog;
};

template <class Impl = TcpPolyServerImpl>
class TcpPolyServerService : public IoService::service {
public:
    static IoService::id id;

    using RequestId = typename Impl::RequestId;
    using RequestPair = typename Impl::RequestPair;

    using RequestHandlerSignature = typename Impl::RequestHandlerSignature;
    using RequestHandler = typename Impl::RequestHandler;

    explicit TcpPolyServerService (IoService& ioService)
        : IoService::service(ioService)
        , mAsyncWork(boost::in_place(std::ref(mAsyncIoService)))
        , mAsyncThread([this] () mutable {
            boost::log::sources::logger log;
            try {
                boost::system::error_code ec;
                auto nHandlers = this->mAsyncIoService.run(ec);
                BOOST_LOG(log) << "TcpPolyServerService: " << nHandlers << " completed with " << ec.message();
            }
            catch (std::exception& e) {
                BOOST_LOG(log) << "TcpPolyServerService died with " << e.what();
            }
            catch (...) {
                BOOST_LOG(log) << "TcpPolyServerService died by unknown cause";
            }
        })
    {}

    ~TcpPolyServerService () {
        mAsyncWork = boost::none;
        mAsyncThread.join();
    }

    using implementation_type = std::shared_ptr<Impl>;

    void construct (implementation_type& impl) {
        impl.reset(new Impl(mAsyncIoService));
    }

    void destroy (implementation_type& impl) {
        impl->destroy();
        impl.reset();
    }

    void close (implementation_type& impl, boost::system::error_code& ec) {
        impl->close(ec);
    }

    void init (implementation_type& impl, Tcp::endpoint endpoint, boost::log::sources::logger log) {
        impl->init(endpoint, log);
    }

    Tcp::endpoint endpoint (const implementation_type& impl) const {
        return impl->endpoint();
    }

    boost::log::sources::logger log (const implementation_type& impl) const {
        return impl->log();
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

    using RequestHandlerSignature = typename Service::RequestHandlerSignature;
    using RequestHandler = typename Service::RequestHandler;

    BasicTcpPolyServer (IoService& ioService, Tcp::endpoint endpoint, boost::log::sources::logger log)
        : boost::asio::basic_io_object<Service>(ioService)
    {
        this->get_service().init(this->get_implementation(), endpoint, log);
    }

    void close () {
        boost::system::error_code ec;
        close(ec);
        if (ec) {
            throw boost::system::system_error(ec);
        }
    }

    void close (boost::system::error_code& ec) {
        this->get_service().close(this->get_implementation(), ec);
    }

    Tcp::endpoint endpoint () const {
        return this->get_service().endpoint(this->get_implementation());
    }

    boost::log::sources::logger log () const {
        return this->get_service().log(this->get_implementation());
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

std::string to_string (TcpPolyServer::RequestId rid) {
    std::ostringstream oss;
    oss << rid.first << "/" << rid.second;
    return oss.str();
}

} // namespace asio
} // namespace rpc

#endif
