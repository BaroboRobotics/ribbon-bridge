#include "rpc/stdlibheaders.hpp"
#include "rpc/message.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

namespace rpc {

template <>
const pb_field_t* pbFields (barobo_rpc_Request) {
    return barobo_rpc_Request_fields;
}

template <>
const pb_field_t* pbFields (barobo_rpc_Reply) {
    return barobo_rpc_Reply_fields;
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

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = barobo_rpc_Request_Type_FIRE;
    request.id = requestId;
    request.has_fire = true;
    request.fire.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            request.fire.payload.bytes,
            sizeof(request.fire.payload.bytes),
            request.fire.payload.size);

    if (!hasError(err)) {
        rpc::encode(request, bytes, size, size, err);
    }
    return err;
}

Status makeBroadcast (uint8_t* bytes, size_t& size, uint32_t componentId, const pb_field_t* fields, void* payload) {
    assert(bytes && fields && payload);

    barobo_rpc_Reply reply;
    memset(&reply, 0, sizeof(reply));

    reply.type = barobo_rpc_Reply_Type_BROADCAST;
    reply.has_inReplyTo = false;
    reply.has_broadcast = true;
    reply.broadcast.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            reply.broadcast.payload.bytes,
            sizeof(reply.broadcast.payload.bytes),
            reply.broadcast.payload.size);

    if (!hasError(err)) {
        rpc::encode(reply, bytes, size, size, err);
    }
    return err;
}

Status makeConnect (uint8_t* bytes, size_t& size, uint32_t requestId) {
    assert(bytes);

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = barobo_rpc_Request_Type_CONNECT;
    request.id = requestId;

    Status status;
    rpc::encode(request, bytes, size, size, status);
    return status;
}

Status makeDisconnect (uint8_t* bytes, size_t& size, uint32_t requestId) {
    assert(bytes);

    barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = barobo_rpc_Request_Type_DISCONNECT;
    request.id = requestId;

    Status status;
    rpc::encode(request, bytes, size, size, status);
    return status;
}

} // namespace rpc
