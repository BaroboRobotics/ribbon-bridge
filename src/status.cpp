#include "rpc/status.hpp"

namespace rpc {

const char* statusToString (Status status) {
#define ITEM(x) case Status::x: return #x
    switch (status) {
        ITEM(OK);
        ITEM(DECODING_FAILURE);
        ITEM(ENCODING_FAILURE);
        ITEM(INCONSISTENT_REQUEST);
        ITEM(INCONSISTENT_REPLY);
        ITEM(ILLEGAL_OPERATION);
        ITEM(NO_SUCH_COMPONENT);
        ITEM(BUFFER_OVERFLOW);
        ITEM(READONLY);
        ITEM(NO_SUBSCRIPTIONS);
        ITEM(VERSION_MISMATCH);

        ITEM(UNSOLICITED_RESULT);
        ITEM(UNRECOGNIZED_RESULT);
        default:
            return "(unknown status)";
    }
#undef ITEM
}

} // namespace rpc
