#ifndef RPC_HASH_HPP
#define RPC_HASH_HPP

#include "rpc/config.hpp"

#include "rpc/stdlibheaders.hpp"

namespace rpc {

/* Paul Larson hash */
template <size_t N>
#if HAVE_CONSTEXPR_FUNCTION_TEMPLATES
constexpr
#endif // HAVE_CONSTEXPR_FUNCTION_TEMPLATES
uint32_t hash (const char (&s)[N], uint32_t seed = 0, size_t i = 0) {
    /* N - 1 so we don't hash the null terminator */
    return (N - 1) == i ? seed : hash(s, seed * 101 + s[i], i + 1);
}

} // namespace rpc

#endif
