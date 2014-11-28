#include "gen-widget.pb.hpp"

#include "sfp/asio/messagequeue.hpp"

#include "rpc/message.hpp"
#include "rpc/version.hpp"
#include "rpc/asio/client.hpp"
#include "rpc/asio/tcppolyserver.hpp"
#include "rpc/asio/forwardcoroutines.hpp"

#include "util/hexdump.hpp"
#include "util/monospawn.hpp"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <functional>
#include <iostream>
#include <string>

using namespace std::placeholders;

using MethodIn = rpc::MethodIn<barobo::Widget>;
using MethodResult = rpc::MethodResult<barobo::Widget>;
using Broadcast = rpc::Broadcast<barobo::Widget>;

using Tcp = boost::asio::ip::tcp;
using TcpMessageQueue = sfp::asio::MessageQueue<Tcp::socket>;
using TcpClient = rpc::asio::Client<TcpMessageQueue>;
using TcpServer = rpc::asio::Server<TcpMessageQueue>;

using UnixDomainSocket = boost::asio::local::stream_protocol::socket;
using UdsMessageQueue = sfp::asio::MessageQueue<UnixDomainSocket>;
using UdsClient = rpc::asio::Client<UdsMessageQueue>;
using UdsServer = rpc::asio::Server<UdsMessageQueue>;

struct WidgetImpl {
    using Interface = barobo::Widget;

    WidgetImpl (std::shared_ptr<UdsServer> server) : mServer(server) {}

    template <class In, class Result = typename rpc::ResultOf<In>::type>
    Result fire (In&& x) {
        return onFire(std::forward<In>(x));
    }

    MethodResult::nullaryNoResult onFire (MethodIn::nullaryNoResult) {
        MethodResult::nullaryNoResult result;
        memset(&result, 0, sizeof(result));
        return result;
    }

    MethodResult::nullaryWithResult onFire (MethodIn::nullaryWithResult) {
        MethodResult::nullaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.value = 2.718281828;
        return result;
    }

    MethodResult::unaryNoResult onFire (MethodIn::unaryNoResult args) {
        MethodResult::unaryNoResult result;
        memset(&result, 0, sizeof(result));
        printf("onFire(unaryNoResult): %f\n", args.value);
        return result;
    }

    MethodResult::unaryWithResult onFire (MethodIn::unaryWithResult args) {
        Broadcast::broadcast broadcast;
        broadcast.value = 1.333;
        asyncBroadcast(*mServer, broadcast, [] (boost::system::error_code ec) {
            if (ec) {
                boost::log::sources::logger log;
                BOOST_LOG(log) << "asyncBroadcast failed with: " << ec.message();
            }
        });
        MethodResult::unaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.value = args.value;
        return result;
    }

    std::shared_ptr<UdsServer> mServer;
};

void proxyCoroutine (std::shared_ptr<UdsClient> client,
    std::shared_ptr<rpc::asio::TcpPolyServer> server,
    boost::asio::yield_context yield) {
    boost::log::sources::logger log;
    try {
        client->messageQueue().asyncHandshake(yield);

        rpc::asio::asyncRunProxy(*client, *server, yield);

        client->messageQueue().asyncShutdown(yield);
        client->messageQueue().stream().close();
    }
    catch (boost::system::system_error& e) {
        BOOST_LOG(log) << "Error in proxy code " << e.what();
    }
}

void serverCoroutine (std::shared_ptr<UdsServer> server,
    boost::asio::yield_context yield) {
    boost::log::sources::logger log;
    try {
        server->messageQueue().asyncHandshake(yield);

        BOOST_LOG(log) << "awaiting connection";
        WidgetImpl widgetImpl { server };
        asyncRunServer(*server, widgetImpl, yield);
        BOOST_LOG(log) << "server " << server.get() << " received disconnection request";
    }
    catch (std::exception& e) {
        BOOST_LOG(log) << "server code threw " << e.what();
    }
    BOOST_LOG(log) << "serverCoroutine exiting";
}



void clientCoroutine (std::shared_ptr<TcpClient> client,
    Tcp::resolver::iterator iter,
    boost::asio::yield_context yield) {
    boost::log::sources::logger log;
    try {
        auto& messageQueue = client->messageQueue();
        auto& stream = messageQueue.stream();

        auto endpoint = boost::asio::async_connect(stream, iter, yield);
        BOOST_LOG(log) << "Connected to " << Tcp::endpoint(*endpoint);
        messageQueue.asyncHandshake(yield);

        auto info = rpc::asio::asyncConnect(*client, std::chrono::milliseconds(500), yield);
        BOOST_LOG(log) << "client connected to server with RPC version "
                       << info.rpcVersion() << ", Interface version "
                       << info.interfaceVersion();

        auto result = rpc::asio::asyncFire(*client, MethodIn::unaryWithResult{2.71828}, std::chrono::milliseconds(500), yield);
        BOOST_LOG(log) << "client fired unaryWithResult(2.71828) -> " << result.value;

        rpc::asio::asyncDisconnect(*client, std::chrono::milliseconds(500), yield);
        BOOST_LOG(log) << "client disconnected";

        messageQueue.asyncShutdown(yield);
        stream.close();
    }
    catch (boost::system::system_error& e) {
        BOOST_LOG(log) << "client code threw " << e.what();
        client->messageQueue().cancel();
    }
    BOOST_LOG(log) << "clientCoroutine exiting";
}




int main (int argc, char** argv) try {
    util::Monospawn sentinel { "baromesh", std::chrono::seconds(1) };
    // TODO initialize logging core
    boost::log::sources::logger log;
    boost::asio::io_service ioService;

#if 0 // TODO
    // Make a deadman switch to stop the daemon if we get an exclusive
    // producer lock.
    util::asio::TmpFileLock producerLock { ioService };
    producerLock.asyncLock([&] (boost::system::error_code ec) {
        if (!ec) {
            BOOST_LOG(log) << "No more baromesh producers, exiting";
            ioService.stop();
        }
    });
#endif

    Tcp::resolver resolver { ioService };
    auto iter = resolver.resolve(std::string("42000"));

    auto server = std::make_shared<UdsServer>(ioService, log);
    auto proxyClient = std::make_shared<UdsClient>(ioService, log);
    auto proxyServer = std::make_shared<rpc::asio::TcpPolyServer>(ioService, log);
    proxyServer->listen(*iter);

    boost::asio::local::connect_pair(
        server->messageQueue().stream(),
        proxyClient->messageQueue().stream());
    boost::asio::spawn(ioService, std::bind(serverCoroutine, server, _1));
    boost::asio::spawn(ioService, std::bind(proxyCoroutine, proxyClient, proxyServer, _1));

    auto nClients = 10;
    if (argc > 1) {
        nClients = std::stoi(argv[1]);
    }

    for (int i = 0; i < nClients; ++i) {
        auto client = std::make_shared<TcpClient>(ioService, log);
        boost::asio::spawn(ioService, std::bind(clientCoroutine, client, iter, _1));
    }

    boost::system::error_code ec;
    auto nHandlers = ioService.run(ec);
    BOOST_LOG(log) << "Event loop executed " << nHandlers << " handlers -- " << ec.message();
}
catch (util::Monospawn::DuplicateProcess& e) {
    boost::log::sources::logger log;
    BOOST_LOG(log) << "baromesh daemon already running, exiting";
}
catch (std::exception& e) {
    boost::log::sources::logger log;
    BOOST_LOG(log) << "baromesh daemon caught some other exception: " << e.what();
}