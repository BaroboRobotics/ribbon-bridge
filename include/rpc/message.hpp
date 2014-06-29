#ifndef RPC_MESSAGE_HPP
#define RPC_MESSAGE_HPP

#include "rpc.pb.h"

#include <pb_encode.h>
#include <pb_decode.h>

#define RPC_MESSAGE_MAX_SIZE 256

namespace rpc {

using Message = com_barobo_rpc_Message;

template <class M>
const pb_field_t* pbFields (M);

bool encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten);
bool decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size);

template <class T>
bool encode (T& message, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    return rpc::encodeProtobuf(&message, pbFields(message), bytes, size, bytesWritten);
}

template <class T>
bool decode (T& message, uint8_t* bytes, size_t size) {
    return rpc::decodeProtobuf(&message, pbFields(message), bytes, size);
}

} // namespace rpc

#endif
