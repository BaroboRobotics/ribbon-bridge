#include "rpc/stdlibheaders.hpp"
#include "rpc/message.hpp"
#include "rpc/proxy.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

namespace rpc {

template <>
const pb_field_t* pbFields (com_barobo_rpc_Request) {
    return com_barobo_rpc_Request_fields;
}

template <>
const pb_field_t* pbFields (com_barobo_rpc_Reply) {
    return com_barobo_rpc_Reply_fields;
}

Error encodeProtobuf (const void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size, size_t& bytesWritten) {
    printf("encoding to buffer of size %zu\n", size);
    auto stream = pb_ostream_from_buffer(bytes, size);
    bool success = true;
    if (!pb_encode(&stream, pbFields, pbStruct)) {
        success = false;
        printf("encoding failed: %s\n", PB_GET_ERROR(&stream));
    }
    else {
        printf("encoded %zu bytes\n", stream.bytes_written);
    }
    bytesWritten = stream.bytes_written;
    return success ?
        Error::NO_ERROR :
        Error::ENCODING_FAILURE;
}

Error decodeProtobuf (void* pbStruct, const pb_field_t* pbFields, uint8_t* bytes, size_t size) {
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
    return success ?
        Error::NO_ERROR :
        Error::DECODING_FAILURE;
}

Error makeGet (uint8_t* bytes, size_t size, uint32_t requestId,
        uint32_t componentId) {
    assert(bytes);

    com_barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = com_barobo_rpc_Request_Type_GET;
    request.id = requestId;
    request.has_get = true;
    request.get.id = componentId;

    return rpc::encode(request, bytes, size, size);
}

Error makeSet (uint8_t* bytes, size_t& size, uint32_t requestId,
        uint32_t componentId, const pb_field_t* fields, void* payload) {
    assert(bytes && fields && payload);

    com_barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = com_barobo_rpc_Request_Type_SET;
    request.id = requestId;
    request.has_set = true;
    request.set.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            request.set.payload.bytes,
            sizeof(request.set.payload.bytes),
            request.set.payload.size);
    if (!hasError(err)) {
        err = rpc::encode(request, bytes, size, size);
    }
    return err;
}

Error makeSubscribe (uint8_t* bytes, size_t& size, uint32_t requestId,
        uint32_t componentId) {
    assert(bytes);

    com_barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = com_barobo_rpc_Request_Type_SUBSCRIBE;
    request.id = requestId;
    request.has_subscribe = true;
    request.subscribe.id = componentId;

    return rpc::encode(request, bytes, size, size);
}

Error makeUnsubscribe (uint8_t* bytes, size_t& size, uint32_t requestId,
        uint32_t componentId) {
    assert(bytes);

    com_barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = com_barobo_rpc_Request_Type_UNSUBSCRIBE;
    request.id = requestId;
    request.has_unsubscribe = true;
    request.unsubscribe.id = componentId;

    return rpc::encode(request, bytes, size, size);
}

Error makeFire (uint8_t* bytes, size_t& size, uint32_t requestId, uint32_t componentId, const pb_field_t* fields, void* payload) {
    assert(bytes && fields && payload);

    com_barobo_rpc_Request request;
    memset(&request, 0, sizeof(request));

    request.type = com_barobo_rpc_Request_Type_FIRE;
    request.id = requestId;
    request.has_fire = true;
    request.fire.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            request.fire.payload.bytes,
            sizeof(request.fire.payload.bytes),
            request.fire.payload.size);

    if (!hasError(err)) {
        err = rpc::encode(request, bytes, size, size);
    }
    return err;
}

Error makeBroadcast (uint8_t* bytes, size_t& size, uint32_t componentId, const pb_field_t* fields, void* payload) {
    assert(bytes && fields && payload);

    com_barobo_rpc_Reply reply;
    memset(&reply, 0, sizeof(reply));

    reply.type = com_barobo_rpc_Reply_Type_BROADCAST;
    reply.has_inReplyTo = false;
    reply.has_broadcast = true;
    reply.broadcast.id = componentId;

    auto err = encodeProtobuf(
            payload, fields,
            reply.broadcast.payload.bytes,
            sizeof(reply.broadcast.payload.bytes),
            reply.broadcast.payload.size);

    if (!hasError(err)) {
        err = rpc::encode(reply, bytes, size, size);
    }
    return err;
}

} // namespace rpc
