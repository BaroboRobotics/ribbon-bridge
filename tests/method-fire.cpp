#include "rpc/stdlibheaders.hpp"

#include "rpc.pb.h"
#include "rpc/service.hpp"
#include "rpc/proxy.hpp"
#include "rpc/message.hpp"
#include "rpc/printmessage.hpp"

#include "rpc/potqueue.hpp"

//////////////////////////////////////////////////////////////////////////////

#include "robot.pb.h"
#include "gen-robot.pb.hpp"
#include "robotimpl.hpp"

void serverSide (uint8_t* buffer, size_t size) {
    RobotImpl robot;

    rpc::Message message;
    rpc::decode(message, buffer, size);

    printf("Received the following message:\n");
    rpc::printMessage(message);

    if (message.has_toObject) {
        if (com_barobo_rpc_ToObject_Type_GET == message.toObject.type ||
                com_barobo_rpc_ToObject_Type_SET == message.toObject.type ||
                com_barobo_rpc_ToObject_Type_FIRE == message.toObject.type) {
            rpc::ComponentUnion<com::barobo::Robot> argument;
            rpc::decodePayload(argument, message.toObject);
            rpc::fire(robot, argument, message.toObject);
        }
        else if (com_barobo_rpc_ToObject_Type_SUBSCRIBE_ATTRIBUTE == message.toObject.type ||
                com_barobo_rpc_ToObject_Type_SUBSCRIBE_BROADCAST == message.toObject.type) {
            //rpc::subscribe(robot, message.toObject);
        }
        else if (com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_ATTRIBUTE == message.toObject.type ||
                com_barobo_rpc_ToObject_Type_UNSUBSCRIBE_BROADCAST == message.toObject.type) {
            //rpc::unsubscribe(robot, message.toObject);
        }
    }

    if (message.has_fromObject) {
        assert(false);
    }

    if (message.has_reply) {
        assert(false);
    }
}

int main () {
    using RobotProxy = rpc::Proxy<com::barobo::Robot>;
    RobotProxy robotProxy;

    robotProxy.move(-234, 8, 1e-3);

    RobotProxy::BufferType buffer;
    auto success = robotProxy.tryPop_(buffer);
    assert(success);

    serverSide(buffer.bytes, buffer.size);
}
