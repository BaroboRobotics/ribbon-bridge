#include "sfp/asio/messagequeue.hpp"

#include "rpc/message.hpp"
#include "rpc/asio/client.hpp"
#include "rpc/asio/server.hpp"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <functional>
#include <iostream>

using namespace std::placeholders;

using Tcp = boost::asio::ip::tcp;
using MessageQueue = sfp::asio::MessageQueue<Tcp::socket>;
using Client = rpc::asio::Client<MessageQueue>;
using Server = rpc::asio::Server<MessageQueue>;

void serverCoroutine (std::shared_ptr<MessageQueue> messageQueue,
	Tcp::endpoint peer,
	boost::asio::yield_context yield)
try {
	messageQueue->asyncHandshake(yield);
	std::cout << "Shook hands with " << peer << '\n';

	std::vector<uint8_t> buf(1024);
	auto size = messageQueue->asyncReceive(boost::asio::buffer(buf), yield);

	barobo_rpc_Request request;
	rpc::decode(request, buf.data(), size);
	if (barobo_rpc_Request_Type_CONNECT == request.type) {
		std::cout << "request was a connect request\n";
	}
	else {
		std::cout << "request was NOT a connect request\n";
	}

	messageQueue->asyncShutdown(yield);
	messageQueue->stream().close();
}
catch (std::exception& e) {
	std::cout << "server code threw " << e.what() << std::endl;
}



void acceptorCoroutine (boost::asio::io_service& ioService,
	Tcp::resolver::iterator iter,
	boost::asio::yield_context yield)
try {
	Tcp::acceptor acceptor { ioService, *iter };
	while (true) {
		auto messageQueue = std::make_shared<MessageQueue>(ioService);
		Tcp::endpoint peer;
		acceptor.async_accept(messageQueue->stream(), peer, yield);
		std::cout << "Accepted connection from " << peer << '\n';

		boost::asio::spawn(ioService, std::bind(serverCoroutine, messageQueue, peer, _1));
	}
}
catch (std::exception& e) {
	std::cout << "acceptor code threw " << e.what() << std::endl;
}



void clientCoroutine (boost::asio::io_service& ioService,
	Tcp::resolver::iterator iter,
	boost::asio::yield_context yield)
try {
	Client client { ioService };
	auto& messageQueue = client.messageQueue();
	auto& stream = messageQueue.stream();

	auto endpoint = boost::asio::async_connect(stream, iter, yield);
	std::cout << "Connected to " << Tcp::endpoint(*endpoint) << '\n';
	messageQueue.asyncHandshake(yield);

	std::vector<uint8_t> buf(10);
	std::iota(buf.begin(), buf.end(), 0);
	messageQueue.asyncSend(boost::asio::buffer(buf), yield);
	auto messageSize = messageQueue.asyncReceive(boost::asio::buffer(buf), yield);
	std::cout << "Received an echo of size " << messageSize << '\n';

	messageQueue.asyncShutdown(yield);
	stream.close();
}
catch (boost::system::system_error& e) {
	std::cout << "client code threw " << e.what() << std::endl;
}



int main () {
	boost::asio::io_service ioService;
	Tcp::resolver resolver { ioService };
	auto iter = resolver.resolve(std::string("42000"));

	boost::asio::spawn(ioService, std::bind(acceptorCoroutine, std::ref(ioService), iter, _1));

	for (int i = 0; i < 10; ++i) {
		boost::asio::spawn(ioService, std::bind(clientCoroutine, std::ref(ioService), iter, _1));
	}

	ioService.run();
}