// Test the rpc::Server class template.

#include "widgetserver.hpp"

#include "rpc/asio/tcpclient.hpp"

#include <boost/asio.hpp>
#include <boost/asio/use_future.hpp>

#include <chrono>
#include <iostream>
#include <future>

namespace asio = boost::asio;

using Tcp = asio::ip::tcp;
using asio::use_future;

void clientThread (asio::io_service& ios, boost::log::sources::logger log) {
    using Method = rpc::MethodIn<barobo::Widget>;
    using rpc::asio::asyncConnect;
    using rpc::asio::asyncDisconnect;
    using rpc::asio::asyncFire;

    auto client = rpc::asio::TcpClient{ios, log};

    auto resolver = Tcp::resolver{ios};
    auto query = Tcp::resolver::query{"127.0.0.1", "12345"};
    asio::connect(client.messageQueue().stream(), resolver.resolve(query));
    client.messageQueue().asyncHandshake(use_future).get();

    auto timeout = std::chrono::seconds{1};

    asyncConnect<barobo::Widget>(client, timeout, use_future).get();

    {
        auto result = asyncFire(client, Method::nullaryNoResult{}, timeout, use_future).get();
        std::cout << "nullary with no result\n";
    }
    {
        auto result = asyncFire(client, Method::nullaryWithResult{}, timeout, use_future).get();
        std::cout << "nullary with result: " << result.value << '\n';
    }
    {
        auto result = asyncFire(client, Method::unaryNoResult{0.5}, timeout, use_future).get();
        std::cout << "unary with no result\n";
    }
    {
        auto result = asyncFire(client, Method::unaryWithResult{0.5}, timeout, use_future).get();
        assert(0.5 == result.value);
        std::cout << "unary with result: " << result.value << '\n';
    }

    asyncDisconnect(client, timeout, use_future).get();
}

enum { SUCCEEDED, FAILED };

int main () try {
    auto testResult = SUCCEEDED;
    auto log = boost::log::sources::logger{};

    // Start up.
    auto ios = asio::io_service{};
    auto iosWork = std::make_shared<asio::io_service::work>(ios);
    auto iosRun = std::async(std::launch::async, [&] { ios.run(); });

    // Construct the acceptor first so we can guarantee the client thread will
    // have something to connec to.
    auto acceptor = Tcp::acceptor{ios, Tcp::endpoint{Tcp::v4(), 12345}};
    auto clientRun = std::async(std::launch::async, [&ios, log] { clientThread(ios, log); });

    sfp::asio::MessageQueue<Tcp::socket> svMq {ios, log};
    acceptor.accept(svMq.stream());
    svMq.asyncHandshake(use_future).get();

    auto server = WidgetServer{[&] (const WidgetServer::BufferType& b) {
        svMq.asyncSend(asio::buffer(b.bytes, b.size), use_future).get();
    }};

    auto buf = WidgetServer::BufferType{};
    // Loop while the client hasn't finished.
    while (std::future_status::timeout == clientRun.wait_for(std::chrono::seconds{0})) {
        buf.size = svMq.asyncReceive(asio::buffer(buf.bytes, sizeof(buf.bytes)), use_future).get();
        auto status = server.receiveProxyBuffer(buf);
        if (hasError(status)) {
            std::cout << "Server error: " << statusToString(status) << std::endl;
            testResult = FAILED;
        }
    }

    // Now shut down.
    clientRun.get();
    iosWork.reset();
    iosRun.get();
    return testResult;
}
catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;;
    return FAILED;
}
