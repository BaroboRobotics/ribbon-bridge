#include "rpc/stdlibheaders.hpp"

#include "rpc.pb.h"
#include "rpc/service.hpp"
#include "rpc/proxy.hpp"
#include "rpc/message.hpp"

//////////////////////////////////////////////////////////////////////////////

#include "robot.pb.h"
#include "gen-robot.pb.hpp"
#include "robotimpl.hpp"

int main () {
    using RobotMethod = rpc::Method<com::barobo::Robot>;

    /* The service would run on the firmware. */
    RobotService robotService;
    /* The proxy would run on the libbarobo host. */
    RobotProxy robotProxy;

    auto invocation = robotProxy.on(RobotMethod::move { -234, 8, 1e-3 });
    assert(!rpc::hasError(invocation.error()));
    RobotProxy::BufferType response;
    auto error = robotService.deliver(invocation.buffer(), response);
    assert(!rpc::hasError(error));
    error = robotProxy.deliver(response);
    assert(!rpc::hasError(error));

    assert(std::future_status::ready == invocation.future().wait_for(std::chrono::seconds(0)));
}

