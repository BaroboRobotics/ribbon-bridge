#include "rpc/stdlibheaders.hpp"

#include "rpc.pb.h"
#include "rpc/service.hpp"
#include "rpc/proxy.hpp"
#include "rpc/message.hpp"

//////////////////////////////////////////////////////////////////////////////

#include "robot.pb.h"
#include "gen-robot.pb.hpp"
#include "robotimpl.hpp"

#include <iostream>

int main () try {
    using RobotMethod = rpc::MethodIn<com::barobo::Robot>;

    /* The service would run on the firmware. */
    RobotService robotService;
    /* The proxy would run on the libbarobo host. */
    RobotProxy robotProxy;

    auto invocation = robotProxy.on(RobotMethod::move { -234, 8, 1e-3 });
    assert(!rpc::hasError(invocation.status()));
    assert(std::future_status::ready != invocation.future().wait_for(std::chrono::seconds(0)));
    RobotProxy::BufferType response;
    auto status = robotService.deliver(invocation.buffer(), response);
    assert(!rpc::hasError(status));
    status = robotProxy.deliver(response);
    assert(!rpc::hasError(status));
    assert(std::future_status::ready == invocation.future().wait_for(std::chrono::seconds(0)));

    const auto& output = invocation.future().get();
    if (output.has_out) {
        std::cout << "robotProxy->move() returned " << output.out.funFactor << '\n';
    }
    else {
        assert(output.has_error);
        std::cout << "robotProxy->move() returned error " << output.error.value << '\n';
    }
}
catch (std::exception& exc) {
    std::cout << exc.what() << '\n';
}
