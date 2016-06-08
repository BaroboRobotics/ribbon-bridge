#include "gen-widget.pb.hpp"

#include "sfp/asio/messagequeue.hpp"

#include "rpc/message.hpp"
#include "rpc/version.hpp"
#include "rpc/asio/client.hpp"
#include "rpc/asio/tcppolyserver.hpp"
#include "rpc/asio/forwardcoroutines.hpp"

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

void startProxyCoroutine (std::shared_ptr<UdsClient> client, std::shared_ptr<rpc::asio::TcpPolyServer> server) {
    client->messageQueue().asyncHandshake([client, server] (boost::system::error_code ec) {
        if (!ec) {
            rpc::asio::asyncRunProxy(*client, *server, [client, server] (boost::system::error_code ec) {
                if (!ec) {
                    client->messageQueue().stream().close();
                }
                else {
                    auto log = client->log();
                    BOOST_LOG(log) << "Proxy run error: " << ec.message();
                }
            });
        }
        else {
            auto log = client->log();
            BOOST_LOG(log) << "Proxy handshake error: " << ec.message();
        }
    });
}

void startServerCoroutine (std::shared_ptr<UdsServer> server) {
    server->messageQueue().asyncHandshake([server] (boost::system::error_code ec) {
        auto log = server->log();
        if (!ec) {
            BOOST_LOG(log) << "awaiting connection";
            auto widgetImpl = std::make_shared<WidgetImpl>(server);
            asyncRunServer(*server, *widgetImpl, [server, widgetImpl] (boost::system::error_code ec) {
                auto log = server->log();
                if (!ec) {
                    BOOST_LOG(log) << "server " << server.get() << " received disconnection request";
                }
                else {
                    BOOST_LOG(log) << "server " << server.get() << " run error: " << ec.message();
                }
            });
        }
        else {
            BOOST_LOG(log) << "server " << server.get() << " handshake error: " << ec.message();
        }
    });
}



void clientCoroutineStepOne (std::shared_ptr<TcpClient> client, boost::system::error_code ec);

void startClientCoroutine (std::shared_ptr<TcpClient> client) {
    //auto f = std::bind(clientCoroutineStepOne, client, _1);
    client->messageQueue().asyncHandshake([client] (boost::system::error_code ec) {
        clientCoroutineStepOne(client, ec);
    });
}

void clientCoroutineStepTwo (std::shared_ptr<TcpClient>, boost::system::error_code, rpc::ServiceInfo);
void clientCoroutineStepOne (std::shared_ptr<TcpClient> client, boost::system::error_code ec) {
    auto log = client->log();
    if (!ec) {
        asyncConnect(*client, std::chrono::milliseconds(500),
            std::bind(&clientCoroutineStepTwo, client, _1, _2));
    }
    else {
        BOOST_LOG(log) << "Client handshake error: " << ec.message();
    }
}

void clientCoroutineStepThree (std::shared_ptr<TcpClient>, boost::system::error_code, MethodResult::unaryWithResult);
void clientCoroutineStepTwo (std::shared_ptr<TcpClient> client, boost::system::error_code ec, rpc::ServiceInfo info) {
    auto log = client->log();
    if (!ec) {
        BOOST_LOG(log) << "client connected to server with RPC version "
                       << info.rpcVersion() << ", Interface version "
                       << info.interfaceVersion();
        auto arg = MethodIn::unaryWithResult{2.71828};
        auto timeout = std::chrono::milliseconds(500);
        asyncFire(*client, arg, timeout,
            std::bind(&clientCoroutineStepThree, client, _1, _2));
    }
    else {
        BOOST_LOG(log) << "Client connect error: " << ec.message();
    }
}

void clientCoroutineStepFour (std::shared_ptr<TcpClient>, boost::system::error_code);
void clientCoroutineStepThree (std::shared_ptr<TcpClient> client, boost::system::error_code ec, MethodResult::unaryWithResult result) {
    auto log = client->log();
    if (!ec) {
        BOOST_LOG(log) << "client fired unaryWithResult(2.71828) -> " << result.value;
        asyncDisconnect(*client, std::chrono::milliseconds(500),
            std::bind(&clientCoroutineStepFour, client, _1));
    }
    else {
        BOOST_LOG(log) << "Client fire error: " << ec.message();
    }
}

void clientCoroutineStepFour (std::shared_ptr<TcpClient> client, boost::system::error_code ec) {
    auto log = client->log();
    if (!ec) {
        BOOST_LOG(log) << "Client disconnected";
        client->messageQueue().stream().close();
    }
    else {
        BOOST_LOG(log) << "Client disconnect error: " << ec.message();
    }
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
    startServerCoroutine(server);
    startProxyCoroutine(proxyClient, proxyServer);

    auto nClients = 10;
    if (argc > 1) {
        nClients = std::stoi(argv[1]);
    }

    for (int i = 0; i < nClients; ++i) {
        auto client = std::make_shared<TcpClient>(ioService, log);
        auto& messageQueue = client->messageQueue();
        auto& stream = messageQueue.stream();

        auto endpoint = boost::asio::connect(stream, iter);
        BOOST_LOG(log) << "Connected to " << Tcp::endpoint(*endpoint);
        startClientCoroutine(client);
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