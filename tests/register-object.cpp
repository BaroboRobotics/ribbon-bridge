#include "stdlibheaders.hpp"

#include "rpc/service.hpp"

/* GENERATED CODE */

namespace com {
namespace barobo {

template <class Derived>
class Robot;

} // namespace barobo
} // namespace com

namespace rpc {

template <>
struct Method<com::barobo::Robot> {
    struct move {
        struct in {
            float desiredAngle1;
            float desiredAngle2;
            float desiredAngle3;
        } in;

        struct out {
            int32_t i;
        } out;

        enum class defaultError {
        } error;

        constexpr static const struct in defaultIn () {
            return {
                float(),
                float(),
                float()
            };
        }

        constexpr static const struct out defaultOut () {
            return {
                int32_t()
            };
        }
    };
};

} // namespace rpc

namespace com {
namespace barobo {

template <class RpcObject>
struct Robot {
    void move (float desiredAngle1, float desiredAngle2, float desiredAngle3) {
        using Method = typename rpc::Method<Robot>::move;
        Method args {
            { desiredAngle1, desiredAngle2, desiredAngle3 },
            Method::defaultOut(),
            Method::defaultError()
        };
        auto& rpcObject = static_cast<RpcObject&>(*this);
        rpc::getInstance(rpcObject).on(args);
    }
};

} // namespace barobo
} // namespace com


/* IMPLEMENTED CODE */

class RobotImpl {
public:
    using RobotMethod = rpc::Method<com::barobo::Robot>;

    /* Magic for attributes and broadcasts goes here. */

    void on (RobotMethod::move args) {
        auto& in = args.in;
        printf("%f %f %f\n", double(in.desiredAngle1),
                double(in.desiredAngle2), double(in.desiredAngle3));
    }

    /* More methods ... */

private:
    /* Implementation details ... */
};

int main () {
    using Robot = rpc::Object<RobotImpl, com::barobo::Robot>;

    Robot robot;
    robot.move(-1, 0, 1);

#if 0
    //robot.accelerometerX().subscribe([] (float value) { });
    //robot.buttonPress().emit(1, 2);

    while (true) {
        //rpcService.runFor(std::chrono::milliseconds(10));
        /* Other stuff ... */
    }
#endif
}
