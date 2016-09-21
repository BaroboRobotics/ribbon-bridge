#include <rpc/status.hpp>

namespace rpc {

#define ITEM(t, x) case t::x: return #x
#define statusToStringBody(t) \
    switch (status) { \
        ITEM(t, OK); \
        ITEM(t, DECODING_FAILURE); \
        ITEM(t, ENCODING_FAILURE); \
        ITEM(t, PROTOCOL_ERROR); \
        ITEM(t, INTERFACE_ERROR); \
        ITEM(t, TIMED_OUT); \
        ITEM(t, VERSION_MISMATCH); \
        default: \
            return "(unknown " #t ")"; \
    }

const char* statusToString (Status status) {
    statusToStringBody(Status)
}

const char* statusToString (RemoteStatus status) {
    statusToStringBody(RemoteStatus)
}

} // namespace rpc
