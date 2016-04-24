#ifndef RPC_ASIO_FORWARDCOROUTINES_HPP
#define RPC_ASIO_FORWARDCOROUTINES_HPP

#include "rpc.pb.h"

#include <util/asio/asynccompletion.hpp>
#include <util/asio/operation.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <chrono>

namespace rpc {
namespace asio {

template <class T, class U>
std::string to_string (const std::pair<T, U> &rid) {
    // PolyServer request IDs are std::pairs, so that they can tack on additional information to
    // route replies to the appropriate subserver. We need to be able to print them.
    std::ostringstream oss;
    oss << rid.first << "/" << rid.second;
    return oss.str();
}

typedef void ForwardRequestsHandlerSignature(boost::system::error_code);
typedef void ForwardBroadcastsHandlerSignature(boost::system::error_code);
typedef void RunProxyHandlerSignature(boost::system::error_code);

template <class C, class S>
struct ForwardOneRequestOperation {
    using SRequestId = typename S::RequestId;
    using SRequestPair = typename S::RequestPair;
    using CRequestId = typename C::RequestId;

    ForwardOneRequestOperation (C& client, S& server, RequestPair rp)
        , client_(client)
        , server_(server)
        , serverRequestId_(rp.first)
        , request_(rp.second)
    {}

    C& client_;
    S& server_;
    RequestId serverRequestId_;
    barobo_rpc_Request request_;

    CRequestId clientRequestId_;

    boost::log::sources::logger log_;

    boost::system::error_code rc_ = boost::asio::error_operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {},
            boost::optional<barobo_rpc_Reply> reply = {}) {
        using boost::log::add_value;
        using std::to_string;
        using rpc::asio::to_string;

        if (!ec) reenter(op) {
            yield client_.asyncSendRequest(requestId_, request_, std::move(op));
            if (barobo_rpc_Request_Type_DISCONNECT == request_.type) {
                client_.close();
                server_.close();
                yield break;
            }
            yield client_.asyncReceiveReply(requestId_, std::chrono::seconds(60), std::move(op));
            if (reply) {
                BOOST_LOG(log_) << add_value("RequestId", to_string(requestId_))
                               << "Forwarding reply to connected client";
                yield server_.asyncSendReply(requestId_, reply, std::move(op));
            }
            else {
                BOOST_LOG(log) << add_value("RequestId", to_string(requestId_))
                               << "Request timed out";
                yield asyncReply(server_, requestId_, Status::TIMED_OUT, std::move(op));
            }
        }
    }
    else if (boost::asio::error::operation_aborted != ec) {
        BOOST_LOG(log_) << "ForwardRequestsOperation I/O error: " << ec.message();
        client_.close();
        server_.close();
        rc_ = ec;
    }
};

template <class C, class S, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::system::error_code))
asyncForwardOneRequest (C& client, S& server, typename S::RequestPair rp,
        CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, void(boost::system::error_code))
    > init { std::forward<CompletionToken>(token) };

    using Forwarder = ForwardOneRequestOperation<C, S>;
    makeOperation<Forwarder>(std::move(init.handler), client, server, rp)();

    return init.result.get();
}

template <class C, class S>
struct ForwardRequestsOperation {
    using RequestId = typename S::RequestId;
    using RequestPair = typename S::RequestPair;

    ForwardRequestsOperation (C& client, S& server)
        , client_(client)
        , server_(server)
    {}

    C& client_;
    S& server_;

    boost::log::sources::logger log_;

    boost::system::error_code rc_ = boost::asio::error_operation_aborted;

    std::tuple<boost::system::error_code> result () const {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {}, RequestPair arg = {}) {
        if (!ec) reenter(op) {
            do {
                yield server_.asyncReceiveRequest(std::move(op));
                fork asyncForwardOneRequest(client, server, rp, op);
            } while (op.is_parent());
        }
        else if (boost::asio::error::operation_aborted != ec) {
            BOOST_LOG(log_) << "ForwardRequestsOperation I/O error: " << ec.message();
            client_.close();
            server_.close();
            rc_ = ec;
        }
    }
};

template <class C, class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, ForwardRequestsHandlerSignature)
asyncForwardRequests (C& client, S& server, Handler&& handler) {
    util::asio::AsyncCompletion<
        Handler, ForwardRequestsHandlerSignature
    > init { std::forward<Handler>(handler) };

    using Op = ForwardRequestsOperation<C, S, decltype(init.handler)>;
    std::make_shared<Op>(client, server)->start(init.handler);

    return init.result.get();
}

template <class C, class S>
struct ForwardBroadcastsOperation {
    ForwardBroadcastsOperation (C& client, S& server)
        : context(client.get_io_service())
        , client_(client)
        , server_(server)
    {}

    C& client_;
    S& server_;

    boost::log::sources::logger log_;
    boost::system::error_code rc_ = boost::asio::error::operation_aborted;

    std::tuple<boost::system::error_code> result () {
        return std::make_tuple(rc_);
    }

    template <class Op>
    void operator() (Op&& op, boost::system::error_code ec = {},
            barobo_rpc_Broadcast broadcast arg = {}) {
        if (!ec) reenter (op) {
            while (true) {
                yield client_.asyncReceiveBroadcast(std::move(op));
                yield server_.asyncSendBroadcast(arg, std::move(op));
            }
        }
        else if (boost::asio::error::operation_aborted != ec) {
            BOOST_LOG(log_) << "ForwardBroadcastsOperation I/O error: " << ec.message();
            client_.close();
            server_.close();
            rc_ = ec;
        }
    }

template <class C, class S, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, ForwardBroadcastsHandlerSignature)
asyncForwardBroadcasts (C& client, S& server, CompletionToken&& token) {
    util::asio::AsyncCompletion<
        CompletionToken, ForwardBroadcastsHandlerSignature
    > init { std::forward<CompletionToken>(token) };

    using Op = ForwardBroadcastsOperation<C, S>;
    util::asio::makeOperation<Op>(std::move(init.handler), client, server)();

    return init.result.get();
}

// Forward requests from the given server to the given client, forward replies
// and broadcasts from the client to the server. Stops only when there is an
// error reported by either the client or the server.
template <class C, class S, class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, RunProxyHandlerSignature)
asyncRunProxy (C& client, S& server, Handler&& handler) {
    util::asio::AsyncCompletion<
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
