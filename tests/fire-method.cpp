#include "robotimpl.hpp"
#include "connectedrpcobject.hpp"

#include <iostream>

int main () {
    enum { SUCCEEDED, FAILED };
    int testResult = SUCCEEDED;

    using Attribute = rpc::Attribute<com::barobo::Robot>;
    using Method = rpc::MethodIn<com::barobo::Robot>;
    using Broadcast = rpc::Broadcast<com::barobo::Robot>;

    ConnectedRpcObject<RobotService, RobotProxy> robot;

    auto moveFuture = robot.proxy().fire(Method::move{-1, 0, 1});

    try {
        auto moveResult = moveFuture.get();
        assert(moveResult.has_out);
        std::cout << "move returned: " << moveResult.out.funFactor << '\n';
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
