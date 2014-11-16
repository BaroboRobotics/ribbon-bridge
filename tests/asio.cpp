#include "gen-widget.pb.hpp"

#include "sfp/asio/messagequeue.hpp"

#include "rpc/message.hpp"
#include "rpc/version.hpp"
#include "rpc/asio/client.hpp"
#include "rpc/asio/tcppolyserver.hpp"

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
            const char* reqType = nullptr;
            switch (request.type) {
                case barobo_rpc_Request_Type_CONNECT:
                    reqType = "CONNECT";
                    break;
                case barobo_rpc_Request_Type_DISCONNECT:
                    reqType = "DISCONNECT";
                    break;
                case barobo_rpc_Request_Type_FIRE:
                    reqType = "FIRE";
                    break;

            }
            const char* repType = nullptr;
            switch (reply.type) {
                case barobo_rpc_Reply_Type_SERVICEINFO:
                    repType = "SERVICEINFO";
                    break;
                case barobo_rpc_Reply_Type_STATUS:
                    repType = "STATUS";
                    break;
                case barobo_rpc_Reply_Type_RESULT:
                    repType = "RESULT";
                    break;
            }
            assert(reqType && repType);
            BOOST_LOG(log) << "proxy forwarded request " << requestId.first << "/" << requestId.second << ": " << std::string(reqType) << " -> " << std::string(repType);
        } while (request.type != barobo_rpc_Request_Type_DISCONNECT);
    }
    catch (boost::system::system_error& e) {
        BOOST_LOG(log) << "Error forwarding requests: " << e.what();
    }
}

void proxyCoroutine (std::shared_ptr<UdsClient> client,
    std::shared_ptr<rpc::asio::TcpPolyServer> server,
    boost::asio::yield_context yield) {
    boost::log::sources::logger log;
    try {
        client->messageQueue().asyncHandshake(yield);

        boost::asio::spawn(yield,
            std::bind(&forwardBroadcastsCoroutine<UdsClient, rpc::asio::TcpPolyServer>,
                std::ref(*client), std::ref(*server), _1));
        forwardRequestsCoroutine(*client, *server, yield);

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
        using S = decltype(server)::element_type;
        S::RequestId requestId;
        barobo_rpc_Request request;
        // Refuse requests with Status::NOT_CONNECTED until we get a CONNECT
        // request. Reply with barobo::Widget's version information.
        std::tie(requestId, request) = processRequestsCoro(*server,
            std::bind(&rpc::asio::rejectIfNotConnectCoro<S>,
                std::ref(*server), _1, _2, _3), yield);

        asyncReply(*server, requestId, rpc::ServiceInfo::create<barobo::Widget>(), yield);
        BOOST_LOG(log) << "server " << server.get() << " connected";

        WidgetImpl widgetImpl { server };

        std::tie(requestId, request) = processRequestsCoro(*server,
            std::bind(&rpc::asio::serveIfNotDisconnectCoro<S, barobo::Widget, WidgetImpl>,
                std::ref(*server), widgetImpl, _1, _2, _3), yield);

        BOOST_LOG(log) << "server " << server.get() << " received disconnection request";
        asyncReply(*server, requestId, rpc::Status::OK, yield);
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

    auto server = std::make_shared<UdsServer>(ioService);
    auto proxyClient = std::make_shared<UdsClient>(ioService);
    auto proxyServer = std::make_shared<rpc::asio::TcpPolyServer>(ioService, *iter);

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
        auto client = std::make_shared<TcpClient>(ioService);
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