#include "stdlibheaders.hpp"

#include "rpc.pb.h"
#include "rpc/object.hpp"
#include "rpc/encode_decode.hpp"
#include "rpc/printmessage.hpp"

#include "potqueue.hpp"

uint32_t getNextMessageId () {
    static uint32_t messageId = 0;
    return messageId++;
}

void buildFire (com_barobo_rpc_Message& message,
        uint32_t objectId,
        uint32_t interfaceId,
        uint32_t elementId,
        uint8_t* bytes, size_t size) {
    /* Payloads are method input structures, and unlikely to need
     * runtime-variable length capacity. This assert should be a static_assert.
     */
    assert(size <= sizeof(message.toObject.payload.bytes));

    memset(&message, 0, sizeof(message));

    message.has_toObject = true;
    message.toObject.type = com_barobo_rpc_ToObject_Type_FIRE;
    message.toObject.messageId = getNextMessageId();
    message.toObject.objectId = objectId;
    message.toObject.interfaceId = interfaceId;
    message.toObject.elementId = elementId;
    memcpy(message.toObject.payload.bytes, bytes, size);
    message.toObject.payload.size = size;
}

//////////////////////////////////////////////////////////////////////////////

#include "robot.pb.h"
#include "gen-robot.pb.hpp"
#include "robotimpl.hpp"

#if 0
    switch (toObject.type) {
        case com_barobo_rpc_ToObject_Type_GET:
            break;
        case com_barobo_rpc_ToObject_Type_SET:
            break;
        case com_barobo_rpc_ToObject_Type_SUBSCRIBE_ATTRIBUTE:
            break;
        case com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_ATTRIBUTE:
            break;
        case com_barobo_rpc_ToObject_Type_SUBSCRIBE_BROADCAST:
            break;
        case com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_BROADCAST:
            break;
        case com_barobo_rpc_ToObject_Type_FIRE:
            break;
        default:
            assert(false);
            break;
    }
}
#endif

void clientSide (uint8_t* buffer, size_t& size) {
    uint8_t argsBuffer[128] = { 0 };
    size_t argsSize = sizeof(argsBuffer);

    /* Build the args */
    com_barobo_Robot_move_In args = {
        1.23,
        4.56,
        7.89
    };

    /* Encode the args */
    rpc::encodeProtobuf(&args, com_barobo_Robot_move_In_fields, argsBuffer, argsSize, argsSize);

    /* Build the message */
    com_barobo_rpc_Message message;
    buildFire(message, 0 /*object*/, 1 /*interface*/, rpc::ComponentId<com::barobo::Robot>::move, argsBuffer, argsSize);

    /* Encode the message */
    rpc::encodeProtobuf(&message, com_barobo_rpc_Message_fields, buffer, size, size);
}

void serverSide (uint8_t* buffer, size_t size) {
    RobotImpl robot;

    com_barobo_rpc_Message message;
    rpc::decodeProtobuf(&message, com_barobo_rpc_Message_fields, buffer, size);

    printf("Received the following message:\n");
    rpc::printMessage(message);

    if (message.has_toObject) {

        /* Check object, interface, and element ids for sanity. */

        rpc::ArgumentUnion<com::barobo::Robot> argument;
        decodeToObjectPayload(argument, message.toObject);
        //rpc::fire(robot, argument);
    }

    if (message.has_fromObject) {
        assert(false);
    }

    if (message.has_reply) {
        assert(false);
    }
}

int main () {
    uint8_t messageBuffer[256];
    size_t size = sizeof(messageBuffer);

    clientSide(messageBuffer, size);
    serverSide(messageBuffer, size);
}
