#ifndef RPC_MESSAGE_HPP
#define RPC_MESSAGE_HPP

#include <rpc/config.hpp>

#ifdef HAVE_EXCEPTIONS
# include <rpc/system_error.hpp>
#endif

#include <rpc/status.hpp>

#include "rpc.pb.h"

#define RPC_MESSAGE_MAX_SIZE 256

namespace rpc {

namespace _ {

// Look up the pb_field_t* of a protobuf given its type (i.e., compile-time).
template <class NanopbStruct>
const pb_field_t* pbFieldPtr ();

// Look up the pb_field_t* of a protobuf given its interface and component ID,
// (i.e., run-time).
template <class Interface>
const pb_field_t* pbFieldPtr (uint32_t);

void encode (const void*, const pb_field_t*, uint8_t*, size_t, pb_size_t&, Status&);
void decode (void*, const pb_field_t*, uint8_t*, size_t size, Status&);

} // namespace _

template <class NanopbStruct>
void encode (const NanopbStruct& message,
    uint8_t* bytes, size_t size,
    pb_size_t& nWritten, Status& status) {
    _::encode(&message, _::pbFieldPtr<NanopbStruct>(), bytes, size, nWritten, status);
}

template <class NanopbStruct>
void decode (NanopbStruct& message,
    uint8_t* bytes, size_t size, Status& status) {
    _::decode(&message, _::pbFieldPtr<NanopbStruct>(), bytes, size, status);
}

#ifdef HAVE_EXCEPTIONS

template <class NanopbStruct>
void encode (const NanopbStruct& message,
    uint8_t* bytes, size_t size,
    pb_size_t& nWritten) {
    Status status;
    encode(message, bytes, size, nWritten, status);
    if (hasError(status)) {
        throw Error(status);
    }
}

template <class NanopbStruct>
void decode (NanopbStruct& message,
    uint8_t* bytes, size_t size) {
    Status status;
    decode(message, bytes, size, status);
    if (hasError(status)) {
        throw Error(status);
    }
}

#endif

} // namespace rpc

#endif
