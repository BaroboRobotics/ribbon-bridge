#ifndef RPC_STATUS_HPP
#define RPC_STATUS_HPP

#include "rpc.pb.h"

namespace rpc {

// if you edit this, make sure you keep statusToString up to date! FIXME use
// macro magic for this.
enum class Status {
    OK                         = barobo_rpc_Status_OK,
    DECODING_FAILURE           = barobo_rpc_Status_DECODING_FAILURE,
    ENCODING_FAILURE           = barobo_rpc_Status_ENCODING_FAILURE,
    INCONSISTENT_REQUEST       = barobo_rpc_Status_INCONSISTENT_REQUEST,
    INCONSISTENT_REPLY         = barobo_rpc_Status_INCONSISTENT_REPLY,
    ILLEGAL_OPERATION          = barobo_rpc_Status_ILLEGAL_OPERATION,
    NO_SUCH_COMPONENT          = barobo_rpc_Status_NO_SUCH_COMPONENT,

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
