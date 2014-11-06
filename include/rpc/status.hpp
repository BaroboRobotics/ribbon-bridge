#ifndef RPC_STATUS_HPP
#define RPC_STATUS_HPP

#include "rpc.pb.h"

namespace rpc {

// if you edit this, make sure you keep statusToString up to date! FIXME use
// macro magic for this.
#define rpc_Status_basic_enumeration(k) \
    OK                         = barobo_rpc_Status_OK + k, \
    DECODING_FAILURE           = barobo_rpc_Status_DECODING_FAILURE + k, \
    ENCODING_FAILURE           = barobo_rpc_Status_ENCODING_FAILURE + k, \
    INCONSISTENT_REQUEST       = barobo_rpc_Status_INCONSISTENT_REQUEST + k, \
    INCONSISTENT_REPLY         = barobo_rpc_Status_INCONSISTENT_REPLY + k, \
    ILLEGAL_OPERATION          = barobo_rpc_Status_ILLEGAL_OPERATION + k, \
    NO_SUCH_COMPONENT          = barobo_rpc_Status_NO_SUCH_COMPONENT + k, \
    NOT_CONNECTED              = barobo_rpc_Status_NOT_CONNECTED + k

enum class Status {
    rpc_Status_basic_enumeration(0),

    UNSOLICITED_REPLY,
    UNRECOGNIZED_RESULT
};

enum class RemoteStatus {
    rpc_Status_basic_enumeration(1)
};

#undef rpc_Status_basic_enumeration

const char* statusToString (Status status);
const char* statusToString (RemoteStatus status);

static_assert(0 == static_cast<int>(Status::OK),
        "Status::OK must have zero value");
static_assert(0 != static_cast<int>(RemoteStatus::OK),
        "RemoteStatus::OK must not have zero value");

static inline bool hasError (Status s) {
    return Status::OK != s;
}

static inline bool hasError (RemoteStatus s) {
    return RemoteStatus::OK != s;
}

} // namespace rpc

#endif
