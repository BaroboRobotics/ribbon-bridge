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

int main () {
    using Queue = boost::lockfree::spsc_queue<RobotProxy::BufferType, boost::lockfree::capacity<16>>;

    Queue proxyToService;
    Queue serviceToProxy;

    RobotService robotService {
        [&] (const RobotService::BufferType& buffer) {
            auto success = serviceToProxy.push(buffer);
            assert(success);
        }
    };

    RobotProxy robotProxy {
        [&] (const RobotProxy::BufferType& buffer) {
            auto success = proxyToService.push(buffer);
            assert(success);
        }
    };

    std::atomic_bool killThreads = { false };

    std::thread serviceToProxyShoveler { [&] () {
        while (!killThreads) {
            serviceToProxy.consume_all([&] (const RobotProxy::BufferType& buffer) {
                auto status = robotProxy.deliver(buffer);
                assert(!hasError(status));
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }};

    std::thread proxyToServiceShoveler { [&] () {
        while (!killThreads) {
            proxyToService.consume_all([&] (const RobotService::BufferType& buffer) {
                auto status = robotService.deliver(buffer);
                assert(!hasError(status));
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }};

    //////////////////////////////////////////////////////////////////////////

    using RobotBroadcast = rpc::Broadcast<com::barobo::Robot>;

    try {
        printf("broadcasting once\n");
        robotService.broadcast(RobotBroadcast::buttonPress { 1, 2 });

        printf("subscribing...\n");
        robotProxy.subscribe(RobotBroadcast::buttonPress()).get();

        printf("broadcasting twice\n");
        robotService.broadcast(RobotBroadcast::buttonPress { 3, 4 });

        printf("unsubscribing...\n");
        robotProxy.unsubscribe(RobotBroadcast::buttonPress()).get();

        printf("broadcasting thrice\n");
        robotService.broadcast(RobotBroadcast::buttonPress { 5, 6 });
    }
    catch (const std::exception& exc) { std::cout << exc.what() << '\n'; }

    using RobotMethod = rpc::MethodIn<com::barobo::Robot>;
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
    proxyToServiceShoveler.join();
    serviceToProxyShoveler.join();
}
