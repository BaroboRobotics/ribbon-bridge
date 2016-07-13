#ifndef RPC_INVOKE_HPP
#define RPC_INVOKE_HPP

#include <rpc/componenttraits.hpp>

#include <boost/system/error_code.hpp>

#include <utility>

namespace rpc {

template <class Interface, class Impl>
void invoke (Impl&& impl, barobo_rpc_Broadcast& broadcast, boost::system::error_code& ec) {
    rpc::BroadcastUnion<Interface> b;
    rpc::Status status;
    b.invoke(std::forward<Impl>(impl), broadcast.id, broadcast.payload, status);
    ec = status;
}

} // rpc

#endif
