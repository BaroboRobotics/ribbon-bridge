#ifndef RPC_ASIO_SERVER_HPP
#define RPC_ASIO_SERVER_HPP

#include <utility>

namespace rpc {
namespace asio {

template <class MessageQueue>
class Server {
public:
	template <class... Args>
	explicit Server (Args&&... args)
		: mMessageQueue(std::forward<Args>(args)...)
	{}

	MessageQueue& messageQueue () { return mMessageQueue; }
	const MessageQueue& messageQueue () const { return mMessageQueue; }

private:
	MessageQueue mMessageQueue;
};

} // namespace asio
} // namespace rpc

#endif