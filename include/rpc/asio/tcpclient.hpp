#ifndef RPC_ASIO_TCPCLIENT_HPP
#define RPC_ASIO_TCPCLIENT_HPP

#include "rpc/asio/client.hpp"
#include "sfp/asio/messagequeue.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <utility>

namespace rpc {
namespace asio {

using Tcp = boost::asio::ip::tcp;
using TcpClient = rpc::asio::Client<sfp::asio::MessageQueue<Tcp::socket>>;

using InitTcpClientHandlerSignature = void(boost::system::error_code);

template <class Handler>
BOOST_ASIO_INITFN_RESULT_TYPE(Handler, InitTcpClientHandlerSignature)
asyncInitTcpClient (TcpClient& client, Tcp::resolver::iterator iter, Handler&& handler) {
    boost::asio::detail::async_result_init<
        Handler, InitTcpClientHandlerSignature
    > init { std::forward<Handler>(handler) };
    auto& realHandler = init.handler;

    boost::asio::async_connect(client.messageQueue().stream(), iter,
        [&client, realHandler] (boost::system::error_code ec, Tcp::resolver::iterator iter) {
            if (!ec) {
                auto log = client.log();
                BOOST_LOG(log) << "TCP client connected to " << iter->endpoint();
                client.messageQueue().asyncHandshake([&client, realHandler] (boost::system::error_code ec) {
                    client.get_io_service().post(std::bind(realHandler, ec));
                });
            }
            else {
                client.get_io_service().post(std::bind(realHandler, ec));
            }
        });

    return init.result.get();
}

} // namespace asio
} // namespace rpc

#endif