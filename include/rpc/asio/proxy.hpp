#ifndef RPC_ASIO_FORWARDCOROUTINES_HPP
#define RPC_ASIO_FORWARDCOROUTINES_HPP

#include "rpc.pb.h"

#include <util/log.hpp>
#include <util/asio/asynccompletion.hpp>
#include <util/asio/operation.hpp>
#include <util/asio/transparentservice.hpp>

#include <boost/asio/io_service.hpp>

#include <boost/log/utility/manipulators/add_value.hpp>

#include <chrono>

#include <boost/asio/yield.hpp>

namespace rpc { namespace asio {

template <class T, class U>
std::string to_string (const std::pair<T, U> &rid) {
    // PolyServer request IDs are std::pairs, so that they can tack on additional information to
    // route replies to the appropriate subserver. We need to be able to print them.
    std::ostringstream oss;
    oss << rid.first << "/" << rid.second;
    return oss.str();
}

template <class Client, class Server>
struct ProxyImpl {
    explicit ProxyImpl (boost::asio::io_service& context)
        : mClient(context)
        , mServer(context)
    {
        mLog.add_attribute("Protocol", boost::log::attributes::constant<std::string>("RB-PX"));
    }

    void close (boost::system::error_code& ec) {
        mClient.close(ec);
        if (ec) {
            BOOST_LOG(mLog) << "Ignoring client close error: " << ec.message();
        }
        mServer.close(ec);
    }

    Client& client () { return mClient; }
    Server& server () { return mServer; }

    auto& log () { return mLog; }

    Client mClient;
    Server mServer;

    mutable util::log::Logger mLog;
};

template <class C, class S>
class Proxy : public util::asio::TransparentIoObject<ProxyImpl<C, S>> {
public:
    using Client = C;
    using Server = S;

    explicit Proxy (boost::asio::io_service& context)
        : util::asio::TransparentIoObject<ProxyImpl<C, S>>(context)
    {}

    Client& client () { return this->get_implementation()->client(); }
    Server& server () { return this->get_implementation()->server(); }

    util::log::Logger& log () { return this->get_implementation()->log(); }
};

typedef void ForwardHandlerSignature(boost::system::error_code);

template <class Proxy>
struct ForwardOneRequestOperation {
    using SRequestPair = typename Proxy::Server::RequestPair;
    using CRequestId = typename Proxy::Client::RequestId;

    explicit ForwardOneRequestOperation (Proxy& proxy, SRequestPair rp)
        : proxy_(proxy)
        , rp_(rp)
    {}

    Proxy& proxy_;

    SRequestPair rp_;

    CRequestId clientRequestId_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {},
            boost::optional<barobo_rpc_Reply> reply = {}) {
        using boost::log::add_value;
        using std::to_string;
        using rpc::asio::to_string;

        if (!ec) reenter (op) {
            clientRequestId_ = proxy_.client().nextRequestId();
            yield proxy_.client().asyncSendRequest(clientRequestId_, rp_.request, std::move(op));
            if (barobo_rpc_Request_Type_DISCONNECT == rp_.request.type) {
                proxy_.close();
                yield break;
            }
            yield proxy_.client().asyncReceiveReply(clientRequestId_,
                std::chrono::seconds(60), std::move(op));
            if (reply) {
                BOOST_LOG(proxy_.log()) << add_value("RequestId", to_string(rp_.id))
                               << "Forwarding reply to connected client";
                yield proxy_.server().asyncSendReply(rp_.id, *reply, std::move(op));
            }
            else {
                BOOST_LOG(proxy_.log()) << add_value("RequestId", to_string(rp_.id))
                               << "Request timed out";
                yield asyncReply(proxy_.server(), rp_.id, Status::TIMED_OUT, std::move(op));
            }
            rc_ = ec;
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
            BOOST_LOG(proxy_.log()) << "ForwardOneRequestOperation I/O error: " << ec.message();
            proxy_.close(ec);
            BOOST_LOG(proxy_.log()) << "Error closing proxy: " << ec.message();
        }
    }
};

template <class Proxy, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::system::error_code))
asyncForwardOneRequest (Proxy& proxy, typename Proxy::Server::RequestPair rp,
        CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, void(boost::system::error_code)
    > init { std::forward<CompletionToken>(token) };

    using Op = ForwardOneRequestOperation<Proxy>;
    util::asio::makeOperation<Op>(std::move(init.handler), proxy, rp)();

    return init.result.get();
}

template <class Proxy>
struct ForwardRequestsOperation {
    using RequestId = typename Proxy::Server::RequestId;
    using RequestPair = typename Proxy::Server::RequestPair;

    explicit ForwardRequestsOperation (Proxy& proxy)
        : proxy_(proxy)
    {}

    Proxy& proxy_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}, RequestPair rp = {}) {
        if (!ec) reenter (op) {
            do {
                yield proxy_.server().asyncReceiveRequest(std::move(op));
                fork asyncForwardOneRequest(proxy_, rp, Op{op});
            } while (op.is_parent());
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
            BOOST_LOG(proxy_.log()) << "ForwardRequestsOperation I/O error: " << ec.message();
            proxy_.close(ec);
            BOOST_LOG(proxy_.log()) << "Error closing proxy: " << ec.message();
        }
    }
};

template <class Proxy, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, ForwardHandlerSignature)
asyncForwardRequests (Proxy& proxy, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, ForwardHandlerSignature
    > init { std::forward<CompletionToken>(token) };

    using Op = ForwardRequestsOperation<Proxy>;
    util::asio::makeOperation<Op>(std::move(init.handler), proxy)();

    return init.result.get();
}

template <class Proxy>
struct ForwardBroadcastsOperation {
    explicit ForwardBroadcastsOperation (Proxy& proxy)
        : proxy_(proxy)
    {}

    Proxy& proxy_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {},
            barobo_rpc_Broadcast arg = {}) {
        if (!ec) reenter (op) {
            while (true) {
                yield proxy_.client().asyncReceiveBroadcast(std::move(op));
                yield proxy_.server().asyncSendBroadcast(arg, std::move(op));
            }
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
            BOOST_LOG(proxy_.log()) << "ForwardBroadcastsOperation I/O error: " << ec.message();
            proxy_.close(ec);
            BOOST_LOG(proxy_.log()) << "Error closing proxy: " << ec.message();
        }
    }
};

template <class Proxy, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, ForwardHandlerSignature)
asyncForwardBroadcasts (Proxy& proxy, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, ForwardHandlerSignature
    > init { std::forward<CompletionToken>(token) };

    using Op = ForwardBroadcastsOperation<Proxy>;
    util::asio::makeOperation<Op>(std::move(init.handler), proxy)();

    return init.result.get();
}

template <class Proxy>
struct RunProxyOperation {
    explicit RunProxyOperation (Proxy& proxy)
        : proxy_(proxy)
    {}

    Proxy& proxy_;

    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}) {
        if (!ec) reenter (op) {
            fork Op{op}();
            if (op.is_child()) {
                yield asyncForwardRequests(proxy_, std::move(op));
            }
            else {
                yield asyncForwardBroadcasts(proxy_, std::move(op));
            }
        }
        else if (boost::asio::error::operation_aborted != ec) {
            rc_ = ec;
            BOOST_LOG(proxy_.log()) << "RunProxyOperation I/O error: " << ec.message();
            proxy_.close(ec);
            BOOST_LOG(proxy_.log()) << "Error closing proxy: " << ec.message();
        }
    }
};

// Forward requests from the given server to the given client, forward replies
// and broadcasts from the client to the server. Stops only when there is an
// error reported by either the client or the server.
template <class Proxy, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, ForwardHandlerSignature)
asyncRunProxy (Proxy& proxy, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, ForwardHandlerSignature
    > init { std::forward<CompletionToken>(token) };

    using Op = RunProxyOperation<Proxy>;
    util::asio::makeOperation<Op>(std::move(init.handler), proxy)();

    return init.result.get();
}

}} // namespace rpc::asio

#include <boost/asio/unyield.hpp>

#endif
