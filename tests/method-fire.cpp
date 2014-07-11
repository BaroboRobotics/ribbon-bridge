#include "rpc/stdlibheaders.hpp"

#include "rpc.pb.h"
#include "rpc/service.hpp"
#include "rpc/proxy.hpp"
#include "rpc/message.hpp"

//////////////////////////////////////////////////////////////////////////////

#include "robot.pb.h"
#include "gen-robot.pb.hpp"
#include "robotimpl.hpp"

#include <boost/lockfree/spsc_queue.hpp>

#include <iostream>

int main () try {
    using RobotMethod = rpc::MethodIn<com::barobo::Robot>;

    using Queue = boost::lockfree::spsc_queue<RobotProxy::BufferType, boost::lockfree::capacity<16>>;

    Queue proxyToService;
    Queue serviceToProxy;

    RobotService robotService;

    RobotProxy robotProxy {
        [&] (const RobotProxy::BufferType& buffer) {
            auto success = proxyToService.push(buffer);
            assert(success);
        }
    };

    std::atomic_bool killThreads = { false };

    auto deliverToService = [&] (const RobotProxy::BufferType& buffer) {
        RobotProxy::BufferType response;
        auto status = robotService.deliver(buffer, response);
        assert(!hasError(status));
        auto success = serviceToProxy.push(response);
        assert(success);
    };

    auto deliverToProxy = [&] (const RobotProxy::BufferType& buffer) {
        auto status = robotProxy.deliver(buffer);
        assert(!hasError(status));
    };

    auto proxyToServiceShoveler = [&] () {
        while (!killThreads) {
            proxyToService.consume_all(deliverToService);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    auto serviceToProxyShoveler = [&] () {
        while (!killThreads) {
            serviceToProxy.consume_all(deliverToProxy);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    std::thread p2st { proxyToServiceShoveler };
    std::thread s2pt { serviceToProxyShoveler };

    //////////////////////////////////////////////////////////////////////////

    auto funFactor = robotProxy.fire(RobotMethod::move { -234, 8, 1e-3 });

    const auto& result = funFactor.get();
    if (result.has_out) {
        std::cout << "robotProxy->move() returned " << result.out.funFactor << '\n';
    }
    else {
        assert(result.has_error);
        std::cout << "robotProxy->move() returned error " << result.error.value << '\n';
    }

    killThreads = true;
    p2st.join();
    s2pt.join();
}
catch (std::exception& exc) {
    std::cout << exc.what() << '\n';
}
