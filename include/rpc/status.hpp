#ifndef RPC_STATUS_HPP
#define RPC_STATUS_HPP

#include "rpc.pb.h"

namespace rpc {

// if you edit this, make sure you keep statusToString up to date! FIXME use
// macro magic for this.
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
    VERSION_MISMATCH        = com_barobo_rpc_Status_VERSION_MISMATCH,

    UNSOLICITED_RESULT,
    UNRECOGNIZED_RESULT
};

const char* statusToString (Status status);

static_assert(0 == static_cast<int>(Status::OK),
        "Status::OK must have zero value");

static inline bool hasError (Status s) {
    return Status::OK != s;
}

} // namespace rpc

#endif
