#ifndef RPC_MESSAGE_HPP
#define RPC_MESSAGE_HPP

#include "rpc/config.hpp"

#ifdef HAVE_EXCEPTIONS
# include "rpc/system_error.hpp"
#endif

#include "rpc/status.hpp"

#include "rpc.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>

#define RPC_MESSAGE_MAX_SIZE 256

namespace rpc {

template <class M>
const pb_field_t* pbFields (M);

Status encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten);
Status decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size);

template <class T>
void encode (const T& message, uint8_t* bytes, size_t size, size_t& bytesWritten, Status& status) {
    status = encodeProtobuf(&message, pbFields(message), bytes, size, bytesWritten);
}

template <class T>
void decode (T& message, uint8_t* bytes, size_t size, Status& status) {
    status = decodeProtobuf(&message, pbFields(message), bytes, size);
}

#ifdef HAVE_EXCEPTIONS

template <class T>
void encode (const T& message, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    Status status;
    encode(message, bytes, size, bytesWritten, status);
    if (hasError(status)) {
        throw Error(status);
    }
}

template <class T>
void decode (T& message, uint8_t* bytes, size_t size) {
    Status status;
    decode(message, bytes, size, status);
    if (hasError(status)) {
        throw Error(status);
    }
}

#endif

Status makeFire (uint8_t* bytes, size_t& size, uint32_t requestId,
        uint32_t componentId, const pb_field_t* fields, void* payload);
Status makeBroadcast (uint8_t* bytes, size_t& size,
        uint32_t componentId, const pb_field_t* fields, void* payload);
Status makeConnect (uint8_t* bytes, size_t& size, uint32_t requestId);
Status makeDisconnect (uint8_t* bytes, size_t& size, uint32_t requestId);

} // namespace rpc

#endif
