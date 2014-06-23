#include "stdlibheaders.hpp"

#include "rpc.pb.h"
#include "potqueue.hpp"

#include <pb_encode.h>
#include <pb_decode.h>

struct Buffer {
    uint8_t buffer[256];
};

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

void onToObject (const com_barobo_rpc_ToObject& toObject) {
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

void onFromObject (const com_barobo_rpc_FromObject& message) {
    (void)message;
}

void onReply (const com_barobo_rpc_Reply& message) {
    (void)message;
}

void onMessage (const com_barobo_rpc_Message& message) {
    if (message.has_toObject) {
        if (message.has_fromObject) {
            printf("Malformed message received: ToObject and FromObject together\n");
        }
        if (message.has_reply) {
            printf("Malformed message received: ToObject and Reply together\n");
        }
        onToObject(message.toObject);
    }
    else if (message.has_fromObject) {
        if (message.has_reply) {
            printf("Malformed message received: FromObject and Reply together\n");
        }
        onFromObject(message.fromObject);
    }
    else if (message.has_reply) {
        onReply(message.reply);
    }
}

//////////////////////////////////////////////////////////////////////////////

uint32_t getNextMessageId () {
    static uint32_t messageId = 0;
    return messageId++;
}

template <class SQ, class BQ>
bool sendSet (SQ& sizeQueue, BQ& bufferQueue) {
    com_barobo_rpc_Message message;
    memset(&message, 0, sizeof(message));

    message.has_toObject = true;
    message.toObject.type = com_barobo_rpc_ToObject_Type_SET;
    message.toObject.messageId = getNextMessageId();
    message.toObject.objectId = 11;
    message.toObject.interfaceId = 22;
    message.toObject.elementId = 33;
    message.toObject.payload.bytes[0] = 0x44;
    message.toObject.payload.bytes[1] = 0x44;
    message.toObject.payload.size = 2;

    onMessage(message);

    Buffer buf;
    auto ostream = pb_ostream_from_buffer(buf.buffer, sizeof(buf));
    bool success = true;
    if (!pb_encode(&ostream, com_barobo_rpc_Message_fields, &message)) {
        success = false;
        printf("encoding failed: %s\n", PB_GET_ERROR(&ostream));
    }
    else {
        printf("encoded %zu bytes\n", ostream.bytes_written);
    }

    sizeQueue.push(ostream.bytes_written);
    bufferQueue.push(buf);

    return success;
}

//////////////////////////////////////////////////////////////////////////////

bool recvRpcMessage (const size_t size, Buffer& buffer) {
    com_barobo_rpc_Message message;
    memset(&message, 0, sizeof(message));

    auto istream = pb_istream_from_buffer(buffer.buffer, size);
    bool success = true;
    auto nBytesLeft = istream.bytes_left;
    if (!pb_decode(&istream, com_barobo_rpc_Message_fields, &message)) {
        success = false;
        printf("decoding failed: %s\n", PB_GET_ERROR(&istream));
    }
    else {
        printf("decoded %zu bytes\n", nBytesLeft - istream.bytes_left);
        onMessage(message);
    }

    return success;
}

int main () {
    PotQueue<Buffer, 2> bufferQueue;
    PotQueue<size_t, 2> sizeQueue;

    sendSet(sizeQueue, bufferQueue);
    recvRpcMessage(sizeQueue.front(), bufferQueue.front());

    sizeQueue.pop();
    bufferQueue.pop();
}
