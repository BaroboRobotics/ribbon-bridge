#ifndef RPC_ERROR_HPP
#define RPC_ERROR_HPP

#include "rpc.pb.h"

namespace rpc {

enum class Error {
    NO_ERROR                = com_barobo_rpc_Error_NO_ERROR,
    DECODING_FAILURE        = com_barobo_rpc_Error_DECODING_FAILURE,
    ENCODING_FAILURE        = com_barobo_rpc_Error_ENCODING_FAILURE,
    INCONSISTENT_REQUEST    = com_barobo_rpc_Error_INCONSISTENT_REQUEST,
    INCONSISTENT_REPLY      = com_barobo_rpc_Error_INCONSISTENT_REPLY,
    ILLEGAL_OPERATION       = com_barobo_rpc_Error_ILLEGAL_OPERATION,
    NO_SUCH_COMPONENT       = com_barobo_rpc_Error_NO_SUCH_COMPONENT,
    BUFFER_OVERFLOW         = com_barobo_rpc_Error_BUFFER_OVERFLOW,
    READ_ONLY               = com_barobo_rpc_Error_READ_ONLY,
    NO_SUBSCRIPTIONS        = com_barobo_rpc_Error_NO_SUBSCRIPTIONS,
    VERSION_MISMATCH        = com_barobo_rpc_Error_VERSION_MISMATCH
};

static_assert(0 == static_cast<int>(Error::NO_ERROR),
        "Error::NO_ERROR must have zero value");

static inline bool hasError (Error e) {
    return Error::NO_ERROR != e;
}

} // namespace rpc

#endif
