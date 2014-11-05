#ifndef RPC_ASIO_CLIENT_HPP
#define RPC_ASIO_CLIENT_HPP

#include <utility>

namespace rpc {
namespace asio {

template <class MessageQueue>
class Client {
public:
	template <class... Args>
	explicit Client (Args&&... args)
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