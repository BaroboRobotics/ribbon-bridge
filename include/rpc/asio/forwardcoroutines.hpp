#ifndef RPC_ASIO_FORWARDCOROUTINES_HPP
#define RPC_ASIO_FORWARDCOROUTINES_HPP

#include "rpc.pb.h"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include <chrono>

namespace rpc {
namespace asio {

template <class C, class S>
void forwardBroadcastsCoroutine (C& client,
    S& server,
    boost::asio::yield_context yield) {
    boost::log::sources::logger log;
    try {
        while (true) {
            auto broadcast = client.asyncReceiveBroadcast(yield);
            server.asyncSendBroadcast(broadcast, yield);
            BOOST_LOG(log) << "Forwarded broadcast";
        }
    }
    catch (boost::system::system_error& e) {
        BOOST_LOG(log) << "Error forwarding broadcasts: " << e.what();
    }
}

template <class C, class S>
void forwardRequestsCoroutine (C& client,
    S& server,
    boost::asio::yield_context yield) {
    boost::log::sources::logger log;
    typename S::RequestId requestId;
    barobo_rpc_Request request;
    try {
        do {
            std::tie(requestId, request) = server.asyncReceiveRequest(yield);
            auto reply = client.asyncRequest(request, std::chrono::seconds(60), yield);
            server.asyncSendReply(requestId, reply, yield);
        } while (request.type != barobo_rpc_Request_Type_DISCONNECT);
    }
    catch (boost::system::system_error& e) {
        BOOST_LOG(log) << "Error forwarding requests: " << e.what();
    }
}

} // namespace asio
} // namespace rpc

#endif