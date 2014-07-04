#ifndef RPC_CHECKVERSION_HPP
#define RPC_CHECKVERSION_HPP

#include "rpc.pb.h"

namespace rpc {

// FIXME hacky
#define RPC_VERSION_MAJOR 0
#define RPC_VERSION_MINOR 0
#define RPC_VERSION_PATCH 0

template <class Interface>
struct Version;

static inline bool checkRpcVersion (const com_barobo_rpc_VersionTriplet& version) {
    return RPC_VERSION_MAJOR == version.major &&
           RPC_VERSION_MINOR == version.minor &&
           RPC_VERSION_PATCH == version.patch;
}

template <class Interface>
bool checkInterfaceVersion (const com_barobo_rpc_VersionTriplet& version) {
    return Version<Interface>::major == version.major &&
           Version<Interface>::minor == version.minor &&
           Version<Interface>::patch == version.patch;
}

} // namespace rpc

#endif
