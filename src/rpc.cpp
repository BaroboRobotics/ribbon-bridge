#include "stdlibheaders.hpp"

#include "rpc/service.hpp"

#include "rpc.pb.h"
#include "potqueue.hpp"

#include <pb_encode.h>
#include <pb_decode.h>

//////////////////////////////////////////////////////////////////////////////

const char* toObjectTypeToString (com_barobo_rpc_ToObject_Type type) {
    switch (type) {
        case com_barobo_rpc_ToObject_Type_GET:
            return "GET";
            break;
        case com_barobo_rpc_ToObject_Type_SET:
            return "SET";
            break;
        case com_barobo_rpc_ToObject_Type_SUBSCRIBE_ATTRIBUTE:
            return "SUBSCRIBE_ATTRIBUTE";
            break;
        case com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_ATTRIBUTE:
            return "UNSUBSCRIBE_ATTRIBUTE";
            break;
        case com_barobo_rpc_ToObject_Type_SUBSCRIBE_BROADCAST:
            return "SUBSCRIBE_BROADCAST";
            break;
        case com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_BROADCAST:
            return "UNSUBSCRIBE_BROADCAST";
            break;
        case com_barobo_rpc_ToObject_Type_FIRE:
            return "FIRE";
            break;
        default:
            return "(unknown message type)";
            break;
    }
}

void printToObject (const com_barobo_rpc_ToObject& toObject) {
    printf("%s {"
            "\n\tmessageId   : %" PRId32
            "\n\tobjectId    : %" PRId32
            "\n\tinterfaceId : %" PRId32
            "\n\telementId   : %" PRId32
            "\n\tpayload     :",
            toObjectTypeToString(toObject.type),
            toObject.messageId,
            toObject.objectId,
            toObject.interfaceId,
            toObject.elementId);

    if (toObject.payload.size) {
        for (size_t i = 0; i < toObject.payload.size; ++i) {
            printf(" %x", int(toObject.payload.bytes[i]));
        }
    }
    else {
        printf(" (empty)");
    }

    printf("\n}\n");
}

void printFromObject (const com_barobo_rpc_FromObject& message) {
    (void)message;
}

void printReply (const com_barobo_rpc_Reply& message) {
    (void)message;
}

void printMessage (const com_barobo_rpc_Message& message) {
    if (message.has_toObject) {
        printToObject(message.toObject);
    }

    if (message.has_fromObject) {
        printFromObject(message.fromObject);
    }

    if (message.has_reply) {
        printReply(message.reply);
    }
}

//////////////////////////////////////////////////////////////////////////////

uint32_t getNextMessageId () {
    static uint32_t messageId = 0;
    return messageId++;
}

template <class Payload>
void buildFire (com_barobo_rpc_Message& message,
        uint32_t objectId,
        uint32_t interfaceId,
        uint32_t elementId,
        const Payload& payload) {
    static_assert(sizeof(payload) <= sizeof(message.toObject.payload.bytes),
            "FIRE payload overflow");

    memset(&message, 0, sizeof(message));

    message.has_toObject = true;
    message.toObject.type = com_barobo_rpc_ToObject_Type_FIRE;
    message.toObject.messageId = getNextMessageId();
    message.toObject.objectId = objectId;
    message.toObject.interfaceId = interfaceId;
    message.toObject.elementId = elementId;
    memcpy(&message.toObject.payload.bytes, &payload, sizeof(payload));
    message.toObject.payload.size = sizeof(payload);
}

bool encodeMessage (rpc::Buffer& buffer, const com_barobo_rpc_Message& message) {
    printMessage(message);

    auto ostream = pb_ostream_from_buffer(buffer.bytes, sizeof(buffer.bytes));
    bool success = true;
    if (!pb_encode(&ostream, com_barobo_rpc_Message_fields, &message)) {
        success = false;
        printf("encoding failed: %s\n", PB_GET_ERROR(&ostream));
    }
    else {
        printf("encoded %zu bytes\n", ostream.bytes_written);
    }

    buffer.size = ostream.bytes_written;
    return success;
}

//////////////////////////////////////////////////////////////////////////////

bool recvRpcMessage (rpc::Buffer& buffer) {
    com_barobo_rpc_Message message;
    memset(&message, 0, sizeof(message));

    auto istream = pb_istream_from_buffer(buffer.bytes, buffer.size);
    bool success = true;
    auto nBytesLeft = istream.bytes_left;
    if (!pb_decode(&istream, com_barobo_rpc_Message_fields, &message)) {
        success = false;
        printf("decoding failed: %s\n", PB_GET_ERROR(&istream));
    }
    else {
        printf("decoded %zu bytes\n", nBytesLeft - istream.bytes_left);
        printMessage(message);
    }

    return success;
}

#include "robot.pb.h"

int main () {
    PotQueue<rpc::Buffer, 2> bufferQueue;

    rpc::Buffer buffer;
    com_barobo_rpc_Message message;
    com_barobo_Robot_MethodmoveIn args = {
        1.23,
        4.56,
        7.89
    };

    buildFire(message, 0 /*object*/, 1 /*interface*/, 2 /*element*/, args);
    encodeMessage(buffer, message);
    bufferQueue.push(buffer);

    recvRpcMessage(bufferQueue.front());
    bufferQueue.pop();
}
