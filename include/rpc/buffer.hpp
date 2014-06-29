#ifndef RPC_BUFFER_HPP
#define RPC_BUFFER_HPP

namespace rpc {

template <size_t N>
struct Buffer {
    uint8_t bytes[N];
    size_t size;
};

} // namespace rpc

#endif
