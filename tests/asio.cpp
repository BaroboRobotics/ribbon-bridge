#include "gen-widget.pb.hpp"

#include "sfp/asio/messagequeue.hpp"

#include "rpc/message.hpp"
#include "rpc/asio/client.hpp"
#include "rpc/asio/server.hpp"

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

using Tcp = boost::asio::ip::tcp;
using MessageQueue = sfp::asio::MessageQueue<Tcp::socket>;
using Client = rpc::asio::Client<MessageQueue>;
using Server = rpc::asio::Server<MessageQueue>;

struct WidgetImpl {
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
        MethodResult::unaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.value = args.value;
        return result;
    }
};

void serverCoroutine (std::shared_ptr<Server> server,
	Tcp::endpoint peer,
	boost::asio::yield_context yield) {
	boost::log::sources::logger log;
	try {
		server->messageQueue().asyncHandshake(yield);

		bool done = false;
		uint32_t requestId;
		barobo_rpc_Request request;
		do {
			std::tie(requestId, request) = server->asyncReceiveRequest(yield);
			switch (request.type) {
				case barobo_rpc_Request_Type_CONNECT:
					BOOST_LOG(log) << "server received a CONNECT";
					{
						barobo_rpc_Reply reply;
						memset(&reply, 0, sizeof(reply));
						reply.type = barobo_rpc_Reply_Type_SERVICEINFO;
						reply.has_serviceInfo = true;
				        reply.serviceInfo.rpcVersion.major = rpc::Version<>::major;
				        reply.serviceInfo.rpcVersion.minor = rpc::Version<>::minor;
				        reply.serviceInfo.rpcVersion.patch = rpc::Version<>::patch;
				        reply.serviceInfo.interfaceVersion.major = rpc::Version<barobo::Widget>::major;
				        reply.serviceInfo.interfaceVersion.minor = rpc::Version<barobo::Widget>::minor;
				        reply.serviceInfo.interfaceVersion.patch = rpc::Version<barobo::Widget>::patch;
				        server->asyncReply(requestId, reply, yield);
					}
					break;
				case barobo_rpc_Request_Type_DISCONNECT:
					BOOST_LOG(log) << "server received a DISCONNECT";
					{
						barobo_rpc_Reply reply;
						memset(&reply, 0, sizeof(reply));
						reply.type = barobo_rpc_Reply_Type_STATUS;
						reply.has_status = true;
						reply.status.value = barobo_rpc_Status_OK;
						server->asyncReply(requestId, reply, yield);
						done = true;
					}
					break;
				case barobo_rpc_Request_Type_FIRE:
					BOOST_LOG(log) << "server received a FIRE";
					{
						// unimplemented
						barobo_rpc_Reply reply;
						memset(&reply, 0, sizeof(reply));
						reply.type = barobo_rpc_Reply_Type_STATUS;
						reply.has_status = true;
						reply.status.value = barobo_rpc_Status_ILLEGAL_OPERATION;
						server->asyncReply(requestId, reply, yield);
					}
					break;
				default:
					throw rpc::Error(rpc::Status::INCONSISTENT_REQUEST);
					break;
			}
		} while (!done);

		server->messageQueue().asyncShutdown(yield);
		server->messageQueue().stream().close();
	}
	catch (std::exception& e) {
		BOOST_LOG(log) << "server code threw " << e.what();
	}
	BOOST_LOG(log) << "serverCoroutine exiting";
}



void acceptorCoroutine (boost::asio::io_service& ioService,
	Tcp::resolver::iterator iter,
	boost::asio::yield_context yield) {
	boost::log::sources::logger log;
	try {
		Tcp::acceptor acceptor { ioService, *iter };
		while (true) {
			auto server = std::make_shared<Server>(ioService);
			Tcp::endpoint peer;
			acceptor.async_accept(server->messageQueue().stream(), peer, yield);
			BOOST_LOG(log) << "Accepted connection from " << peer;
			boost::asio::spawn(ioService, std::bind(serverCoroutine, server, peer, _1));
		}
	}
	catch (std::exception& e) {
		BOOST_LOG(log) << "acceptor code threw " << e.what();
	}
	BOOST_LOG(log) << "acceptorCoroutine exiting";
}



void clientCoroutine (boost::asio::io_service& ioService,
	Tcp::resolver::iterator iter,
	boost::asio::yield_context yield) {
	boost::log::sources::logger log;
	Client client { ioService };
	try {
		auto& messageQueue = client.messageQueue();
		auto& stream = messageQueue.stream();

		auto endpoint = boost::asio::async_connect(stream, iter, yield);
		BOOST_LOG(log) << "Connected to " << Tcp::endpoint(*endpoint);
		messageQueue.asyncHandshake(yield);

		auto info = rpc::asio::asyncConnect(client, std::chrono::milliseconds(500), yield);
		BOOST_LOG(log) << "client connected to server with RPC version "
					   << info.rpcVersion() << ", Interface version "
					   << info.interfaceVersion();

		rpc::asio::asyncDisconnect(client, std::chrono::milliseconds(500), yield);
		BOOST_LOG(log) << "client disconnected";

		messageQueue.asyncShutdown(yield);
		stream.close();
	}
	catch (boost::system::system_error& e) {
		BOOST_LOG(log) << "client code threw " << e.what();
		client.messageQueue().cancel();
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

	boost::asio::spawn(ioService, std::bind(acceptorCoroutine, std::ref(ioService), iter, _1));

	auto nClients = 10;
	if (argc > 1) {
		nClients = std::stoi(argv[1]);
	}

	for (int i = 0; i < nClients; ++i) {
		boost::asio::spawn(ioService, std::bind(clientCoroutine, std::ref(ioService), iter, _1));
	}

	ioService.run();
}
catch (util::Monospawn::DuplicateProcess& e) {
	boost::log::sources::logger log;
	BOOST_LOG(log) << "baromesh daemon already running, exiting";
}
catch (std::exception& e) {
	boost::log::sources::logger log;
	BOOST_LOG(log) << "baromesh daemon caught some other exception: " << e.what();
}