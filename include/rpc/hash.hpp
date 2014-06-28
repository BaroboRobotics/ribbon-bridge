#ifndef RPC_HASH_HPP
#define RPC_HASH_HPP

#include "rpc/stdlibheaders.hpp"

namespace rpc {

/* Paul Larson hash */
template <size_t N>
constexpr uint32_t hash (const char (&s)[N], uint32_t seed = 0, size_t i = 0) {
    return N == i ? seed : hash(s, seed * 101 + s[i], i + 1);
}

} // namespace rpc

#endif
