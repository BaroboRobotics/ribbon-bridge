#include "stdlibheaders.hpp"

#include "rpc/service.hpp"

/* GENERATED CODE */

namespace com {
namespace barobo {

template <typename Impl>
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

template <typename Impl>
class Robot {

public:
    Robot (Impl& impl) : mImpl(impl) { }

    void move (float desiredAngle1, float desiredAngle2, float desiredAngle3) {
        using method = typename rpc::Method<Robot>::move;
        method args {
            { desiredAngle1, desiredAngle2, desiredAngle3 },
            method::defaultOut(),
            method::defaultError()
        };
        mImpl.on(args);
    }

private:
    Impl& mImpl;
};

} // namespace barobo
} // namespace com


/* IMPLEMENTED CODE */

class RobotImpl {
public:
    using Interface = com::barobo::Robot<RobotImpl>;
    using Method = rpc::Method<com::barobo::Robot>;

    RobotImpl (Interface& self) : mSelf(self) { }

    /* Magic for attributes and broadcasts goes here. */

    void on (Method::move args) {
        auto& in = args.in;
        printf("%f %f %f\n", double(in.desiredAngle1),
                double(in.desiredAngle2), double(in.desiredAngle3));
    }

    /* More methods ... */

private:
    Interface& mSelf;
};


int main () {
    rpc::Service rpcService;
    rpc::Object<RobotImpl> robot;
    rpcService.registerObject(robot);

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
