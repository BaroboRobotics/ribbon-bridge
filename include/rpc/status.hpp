#ifndef RPC_STATUS_HPP
#define RPC_STATUS_HPP

#include "rpc.pb.h"

namespace rpc {

// if you edit this, make sure you keep statusToString up to date! FIXME use
// macro magic for this.
#define rpc_Status_basic_enumeration \
    OK                         = barobo_rpc_Status_OK, \
    DECODING_FAILURE           = barobo_rpc_Status_DECODING_FAILURE, \
    ENCODING_FAILURE           = barobo_rpc_Status_ENCODING_FAILURE, \
    MESSAGE_SANITY_FAILURE     = barobo_rpc_Status_MESSAGE_SANITY_FAILURE, \
    NO_SUCH_COMPONENT          = barobo_rpc_Status_NO_SUCH_COMPONENT, \
    NOT_CONNECTED              = barobo_rpc_Status_NOT_CONNECTED, \
    CONNECTION_REFUSED         = barobo_rpc_Status_CONNECTION_REFUSED, \
    TIMED_OUT                  = barobo_rpc_Status_TIMED_OUT

enum class Status {
    rpc_Status_basic_enumeration,

    VERSION_MISMATCH
};

enum class RemoteStatus {
    rpc_Status_basic_enumeration
};

#undef rpc_Status_basic_enumeration

const char* statusToString (Status status);
const char* statusToString (RemoteStatus status);

static_assert(0 == static_cast<int>(Status::OK),
        "Status::OK must have zero value");
static_assert(0 == static_cast<int>(RemoteStatus::OK),
        "RemoteStatus::OK must have zero value");

static inline bool hasError (Status s) {
    return Status::OK != s;
}

static inline bool hasError (RemoteStatus s) {
    return RemoteStatus::OK != s;
}

} // namespace rpc

#endif
