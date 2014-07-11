#ifndef RPC_ERROR_HPP
#define RPC_ERROR_HPP

#include "rpc.pb.h"

namespace rpc {

enum class Status {
    OK                      = com_barobo_rpc_Status_OK,
    DECODING_FAILURE        = com_barobo_rpc_Status_DECODING_FAILURE,
    ENCODING_FAILURE        = com_barobo_rpc_Status_ENCODING_FAILURE,
    INCONSISTENT_REQUEST    = com_barobo_rpc_Status_INCONSISTENT_REQUEST,
    INCONSISTENT_REPLY      = com_barobo_rpc_Status_INCONSISTENT_REPLY,
    ILLEGAL_OPERATION       = com_barobo_rpc_Status_ILLEGAL_OPERATION,
    NO_SUCH_COMPONENT       = com_barobo_rpc_Status_NO_SUCH_COMPONENT,
    BUFFER_OVERFLOW         = com_barobo_rpc_Status_BUFFER_OVERFLOW,
    READ_ONLY               = com_barobo_rpc_Status_READ_ONLY,
    NO_SUBSCRIPTIONS        = com_barobo_rpc_Status_NO_SUBSCRIPTIONS,
    VERSION_MISMATCH        = com_barobo_rpc_Status_VERSION_MISMATCH
};

static_assert(0 == static_cast<int>(Status::OK),
        "Status::OK must have zero value");

static inline bool hasError (Status s) {
    return Status::OK != s;
}

} // namespace rpc

#endif
