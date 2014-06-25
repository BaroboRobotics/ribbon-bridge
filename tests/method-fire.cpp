#include "stdlibheaders.hpp"

#include "rpc.pb.h"
#include "rpc/object.hpp"
#include "rpc/buffer.hpp"
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
        rpc::Buffer& payload) {
    /* Payloads are method input structures, and unlikely to need
     * runtime-variable length capacity. This assert should be a static_assert.
     */
    assert(payload.size() <= sizeof(message.toObject.payload.bytes));

    memset(&message, 0, sizeof(message));

    message.has_toObject = true;
    message.toObject.type = com_barobo_rpc_ToObject_Type_FIRE;
    message.toObject.messageId = getNextMessageId();
    message.toObject.objectId = objectId;
    message.toObject.interfaceId = interfaceId;
    message.toObject.elementId = elementId;
    memcpy(&message.toObject.payload.bytes, payload.bytes(), payload.size());
    message.toObject.payload.size = payload.size();
}

//////////////////////////////////////////////////////////////////////////////

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

/* No inheritance is necessary, call the class whatever you want. */
class RobotImpl {
public:
    /* This typedef isn't required, but it makes things more readable. If you
     * implement multiple interfaces, you might make multiple typedefs. */
    using RobotMethod = rpc::Method<com::barobo::Robot>;

    /* TODO: Magic for attributes and broadcasts goes here. */

    /* Methods are implemented by overloading the "on" member function, and
     * taking the interface method structure (containing input and output
     * parameter structures, and an error field) as the single reference
     * parameter. */
    void on (RobotMethod::move& args) {
        auto& in = args.in;
        printf("%f %f %f\n", double(in.desiredAngle1),
                double(in.desiredAngle2), double(in.desiredAngle3));
    }

    /* More methods ... */

private:
    /* Implementation details ... */
};

#include "robot.pb.h"

template <class O>
void deliverMessageToObject (O& object, const com_barobo_rpc_ToObject& toObject) {
    switch (toObject.type) {
        case com_barobo_rpc_ToObject_Type_GET:
            assert(false);
            break;
        case com_barobo_rpc_ToObject_Type_SET:
            assert(false);
            break;
        case com_barobo_rpc_ToObject_Type_SUBSCRIBE_ATTRIBUTE:
            assert(false);
            break;
        case com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_ATTRIBUTE:
            assert(false);
            break;
        case com_barobo_rpc_ToObject_Type_SUBSCRIBE_BROADCAST:
            assert(false);
            break;
        case com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_BROADCAST:
            assert(false);
            break;
        case com_barobo_rpc_ToObject_Type_FIRE:
            //fire(object, toObject.payload.bytes, toObject.payload.size);
            break;
        default:
            assert(false);
            break;
    }
}

int main () {
    rpc::Buffer messageBuffer;

    //////////////////////////////////////////////////////////////////////////
    /* Client side */
    {
        rpc::Buffer argsBuffer;

        /* Build the args */
        com_barobo_Robot_MethodmoveIn args = {
            1.23,
            4.56,
            7.89
        };

        /* Encode the args */
        argsBuffer.encode(args, com_barobo_Robot_MethodmoveIn_fields);

        /* Build the message */
        com_barobo_rpc_Message message;
        buildFire(message, 0 /*object*/, 1 /*interface*/, 2 /*element*/, argsBuffer);

        /* Encode the message */
        messageBuffer.encode(message, com_barobo_rpc_Message_fields);
    }

    //////////////////////////////////////////////////////////////////////////
    /* Server side */
    {
        com_barobo_rpc_Message message;
        messageBuffer.decode(message, com_barobo_rpc_Message_fields);

        printf("Received the following message:\n");
        rpc::printMessage(message);

        if (message.has_toObject) {
            using Robot = rpc::Object<RobotImpl, com::barobo::Robot>;
            Robot robot;

            deliverMessageToObject(robot, message.toObject);
        }

        if (message.has_fromObject) {
            assert(false);
        }

        if (message.has_reply) {
            assert(false);
        }
    }
}
