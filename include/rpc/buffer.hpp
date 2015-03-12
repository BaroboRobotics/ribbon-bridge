#ifndef RPC_BUFFER_HPP
#define RPC_BUFFER_HPP

//#include "rpc.pb.h"

namespace rpc {

template <size_t N>
struct Buffer {
    uint8_t bytes[N];
    pb_size_t size;
};

} // namespace rpc

#endif
