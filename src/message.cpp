#include "rpc/stdlibheaders.hpp"
#include "rpc/message.hpp"
#include "rpc/proxy.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

namespace rpc {

template <>
const pb_field_t* pbFields (com_barobo_rpc_Message) {
    return com_barobo_rpc_Message_fields;
};

bool encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    printf("encoding to buffer of size %zu\n", size);
    auto stream = pb_ostream_from_buffer(bytes, size);
    bool success = true;
    if (!pb_encode(&stream, pbFields, pbStruct)) {
        success = false;
        printf("encoding failed: %s\n", PB_GET_ERROR(&stream));
    }
    else {
    }
        printf("encoded %zu bytes\n", stream.bytes_written);
    bytesWritten = stream.bytes_written;
    return success;
}

bool decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size) {
    auto stream = pb_istream_from_buffer(bytes, size);
    bool success = true;
    auto nBytesLeft = stream.bytes_left;
    if (!pb_decode(&stream, pbFields, pbStruct)) {
        success = false;
        printf("decoding failed: %s\n", PB_GET_ERROR(&stream));
    }
    else {
        printf("decoded %zu bytes\n", nBytesLeft - stream.bytes_left);
    }
    return success;
}

bool makeMessage (uint8_t* buffer, size_t& size, com_barobo_rpc_ToObject_Type type, uint32_t messageId, uint32_t componentId, const pb_field_t* fields, void* args) {
    Message message;
    memset(&message, 0, sizeof(message));

    message.has_toObject = true;
    message.toObject.type = type;
    message.toObject.messageId = messageId;
    message.toObject.componentId = componentId;

    /* Encode the args, if any. */
    bool success = true;
    if (fields && args) {
        success = encodeProtobuf(
                args, fields,
                message.toObject.payload.bytes,
                sizeof(message.toObject.payload.bytes),
                message.toObject.payload.size);
    }

    return success && rpc::encode(message, buffer, size, size);
}

} // namespace rpc
