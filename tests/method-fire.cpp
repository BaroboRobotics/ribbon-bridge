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
    RobotService robot;

    rpc::Request request;
    rpc::decode(request, buffer, size);

    printf("Received the following message:\n");
    rpc::printRequest(request);

    if (request.has_component) {
        if (request.component.has_invocation) {
            rpc::ComponentUnion<com::barobo::Robot> argument;
            rpc::decodePayload(argument, request.component.id, request.component.invocation);
            rpc::invoke(robot, argument, request.component.id, request.component.invocation);
        }
    }
}

int main () {
    RobotProxy robotProxy;

    robotProxy->move(-234, 8, 1e-3);

    RobotProxy::BufferType buffer;
    auto success = robotProxy.tryPop(buffer);
    assert(success);

    serverSide(buffer.bytes, buffer.size);
}
