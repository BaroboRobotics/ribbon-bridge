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

int main () {
    RobotProxy robotProxy;
    robotProxy->move(-234, 8, 1e-3);

    RobotProxy::BufferType buffer;
    auto success = robotProxy.tryPop(buffer);
    assert(success);

    RobotService robot;
    robot.deliver(buffer);

}
