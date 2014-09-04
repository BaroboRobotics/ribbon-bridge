#ifndef RPC_MESSAGE_HPP
#define RPC_MESSAGE_HPP

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
Status encode (const T& message, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    return encodeProtobuf(&message, pbFields(message), bytes, size, bytesWritten);
}

template <class T>
Status decode (T& message, uint8_t* bytes, size_t size) {
    return decodeProtobuf(&message, pbFields(message), bytes, size);
}

Status makeFire (uint8_t* bytes, size_t& size, uint32_t requestId,
        uint32_t componentId, const pb_field_t* fields, void* payload);
Status makeBroadcast (uint8_t* bytes, size_t& size,
        uint32_t componentId, const pb_field_t* fields, void* payload);
Status makeConnect (uint8_t* bytes, size_t& size, uint32_t requestId);

} // namespace rpc

#endif
