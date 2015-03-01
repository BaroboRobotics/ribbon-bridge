#include "rpc/stdlibheaders.hpp"
#include "rpc/message.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

namespace rpc {
namespace _ {

template <>
const pb_field_t* pbFieldPtr<barobo_rpc_ClientMessage> () {
    return barobo_rpc_ClientMessage_fields;
}

template <>
const pb_field_t* pbFieldPtr<barobo_rpc_ServerMessage> () {
    return barobo_rpc_ServerMessage_fields;
}

void encode (const void* pbStruct, const pb_field_t* pbFields,
    uint8_t* bytes, size_t size,
    size_t& nWritten, Status& status) {
    auto stream = pb_ostream_from_buffer(bytes, size);
    status = Status::OK;
    if (!pb_encode(&stream, pbFields, pbStruct)) {
        status = Status::ENCODING_FAILURE;
    }
    nWritten = stream.bytes_written;
}

void decode (void* pbStruct, const pb_field_t* pbFields,
    uint8_t* bytes, size_t size, Status& status) {
    auto stream = pb_istream_from_buffer(bytes, size);
    status = Status::OK;
    if (!pb_decode(&stream, pbFields, pbStruct)) {
        status = Status::DECODING_FAILURE;
    }
}

} // namespace _
} // namespace rpc
