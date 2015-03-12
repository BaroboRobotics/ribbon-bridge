#ifndef RPC_ASIO_TCPSERVER_HPP
#define RPC_ASIO_TCPSERVER_HPP

#include "rpc/asio/server.hpp"
#include "sfp/asio/messagequeue.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <utility>

namespace rpc {
namespace asio {

using Tcp = boost::asio::ip::tcp;
using TcpServer = rpc::asio::Server<sfp::asio::MessageQueue<Tcp::socket>>;

typedef void InitTcpServerHandlerSignature(boost::system::error_code, Tcp::endpoint);

template <class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, InitTcpServerHandlerSignature)
asyncInitTcpServer (TcpServer& server, Tcp::acceptor& acceptor, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, InitTcpServerHandlerSignature
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    auto peer = std::make_shared<Tcp::endpoint>();
    acceptor.async_accept(server.messageQueue().stream(), *peer,
        [&server, realHandler, peer] (boost::system::error_code ec) {
            if (!ec) {
                auto log = server.log();
                BOOST_LOG(log) << "TCP server connected to " << *peer;
                server.messageQueue().asyncHandshake([&server, realHandler, peer] (boost::system::error_code ec) {
                    server.get_io_service().post(std::bind(realHandler, ec, *peer));
                });
            }
            else {
                server.get_io_service().post(std::bind(realHandler, ec, *peer));
            }
        });

    return init.result.get();
}

} // namespace asio
} // namespace rpc

#endif