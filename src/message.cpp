#include "rpc/stdlibheaders.hpp"
#include "rpc/message.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

namespace rpc {

template <>
const pb_field_t* pbFields (barobo_rpc_ClientMessage) {
    return barobo_rpc_ClientMessage_fields;
}

template <>
const pb_field_t* pbFields (barobo_rpc_ServerMessage) {
    return barobo_rpc_ServerMessage_fields;
}

Status encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    auto stream = pb_ostream_from_buffer(bytes, size);
    bool success = true;
    if (!pb_encode(&stream, pbFields, pbStruct)) {
        success = false;
    }
    bytesWritten = stream.bytes_written;
    return success ?
        Status::OK :
        Status::ENCODING_FAILURE;
}

Status decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size) {
    auto stream = pb_istream_from_buffer(bytes, size);
    bool success = true;
    if (!pb_decode(&stream, pbFields, pbStruct)) {
        success = false;
    }
    return success ?
        Status::OK :
        Status::DECODING_FAILURE;
}

Status makeFire (uint8_t* bytes, size_t& size, uint32_t requestId, uint32_t componentId, const pb_field_t* fields, void* payload) {
    assert(bytes && fields && payload);

    barobo_rpc_ClientMessage message;
    memset(&message, 0, sizeof(message));

    message.id = requestId;
    message.request.type = barobo_rpc_Request_Type_FIRE;
    message.request.has_fire = true;
    message.request.fire.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            message.request.fire.payload.bytes,
            sizeof(message.request.fire.payload.bytes),
            message.request.fire.payload.size);

    if (!hasError(err)) {
        rpc::encode(message, bytes, size, size, err);
    }
    return err;
}

Status makeBroadcast (uint8_t* bytes, size_t& size, uint32_t componentId, const pb_field_t* fields, void* payload) {
    assert(bytes && fields && payload);

    barobo_rpc_ServerMessage message;
    memset(&message, 0, sizeof(message));

    message.type = barobo_rpc_ServerMessage_Type_BROADCAST;
    message.has_inReplyTo = false;
    message.has_broadcast = true;
    message.broadcast.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            message.broadcast.payload.bytes,
            sizeof(message.broadcast.payload.bytes),
            message.broadcast.payload.size);

    if (!hasError(err)) {
        rpc::encode(message, bytes, size, size, err);
    }
    return err;
}

Status makeConnect (uint8_t* bytes, size_t& size, uint32_t requestId) {
    assert(bytes);

    barobo_rpc_ClientMessage message;
    memset(&message, 0, sizeof(message));

    message.id = requestId;
    message.request.type = barobo_rpc_Request_Type_CONNECT;
    message.request.has_fire = false;

    Status status;
    rpc::encode(message, bytes, size, size, status);
    return status;
}

Status makeDisconnect (uint8_t* bytes, size_t& size, uint32_t requestId) {
    assert(bytes);

    barobo_rpc_ClientMessage message;
    memset(&message, 0, sizeof(message));

    message.id = requestId;
    message.request.type = barobo_rpc_Request_Type_DISCONNECT;
    message.request.has_fire = false;

    Status status;
    rpc::encode(message, bytes, size, size, status);
    return status;
}

} // namespace rpc
