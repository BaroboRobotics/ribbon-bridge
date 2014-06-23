/* GENERATED CODE (protobuf) */

#ifndef fartknockers
#define fartknockers

#include "rpc.pb.h"

#include "rpc/service.hpp"

namespace rpc {
namespace extensions {
namespace com {
namespace barobo {

union Robot {
    /* List of all possible types used in this interface */
    int32_t val_int32;
    float val_float;
};

} // namespace barobo
} // namespace com
} // namespace extensions

template <>
pb_extension_t& getStaticExtensionChain<extensions::com::barobo::Robot> () {
    auto& u = getStaticExtensionUnion<extensions::com::barobo::Robot>();

    /* Reverse order! Start with the end of the linked list so we can
     * initialize the next pointers. */
    static pb_extension_t ext_float = {
        &UserValue_float,
        &u.val_float,
        nullptr,
        false
    };

    static pb_extension_t ext_int32 = {
        &UserValue_int32,
        &u.val_int32,
        &ext_float,
        false
    };

    return ext_int32;
}

} // namespace rpc

#endif
