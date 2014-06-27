#include "rpc/stdlibheaders.hpp"
#include "rpc/printmessage.hpp"

namespace rpc {

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
            "\n\tcomponentId : %" PRId32
            "\n\tpayload     :",
            toObjectTypeToString(toObject.type),
            toObject.messageId,
            toObject.objectId,
            toObject.interfaceId,
            toObject.componentId);

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

} // namespace rpc
