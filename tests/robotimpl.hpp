#ifndef RPC_TESTS_ROBOTIMPL_HPP
#define RPC_TESTS_ROBOTIMPL_HPP

#include "rpc/service.hpp"
#include "rpc/asyncproxy.hpp"

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

class RobotService : public rpc::Service<RobotService, com::barobo::Robot> {
public:
    RobotService (std::function<void(const BufferType&)> postFunc) : mPostFunc(postFunc) { }

    void post (const BufferType& buffer) {
        mPostFunc(buffer);
    }

    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using MethodIn = rpc::MethodIn<com::barobo::Robot>;
    using MethodResult = rpc::MethodResult<com::barobo::Robot>;
    using Attribute = rpc::Attribute<com::barobo::Robot>;

    Attribute::motorPower get (Attribute::motorPower) {
        return motorPower;
    }

    void set (Attribute::motorPower args) {
        motorPower = args;
    }

    /* Methods are implemented by overloading the "on_" member function, and
     * taking the interface method structure (containing input and output
     * parameter structures, and an error field) as the single reference
     * parameter. */
    MethodResult::move fire (MethodIn::move& args) {
        printf("%f %f %f\n", double(args.desiredAngle1),
                double(args.desiredAngle2), double(args.desiredAngle3));
        MethodResult::move result;
        result.has_out = true;
        result.out.funFactor = 1.23;
        return result;
    }

    /* More methods ... */

private:
    Attribute::motorPower motorPower;

    std::function<void(const BufferType&)> mPostFunc;
};

class RobotProxy : public rpc::AsyncProxy<RobotProxy, com::barobo::Robot> {
public:
    RobotProxy (std::function<void(const BufferType&)> postFunc) : mPostFunc(postFunc) { }

    void post (const BufferType& buffer) {
        mPostFunc(buffer);
    }

    using Attribute = rpc::Attribute<com::barobo::Robot>;
    using Broadcast = rpc::Broadcast<com::barobo::Robot>;

    void broadcast (Attribute::motorPower& args) {
        printf("motorPower! %" PRId32 "\n", args.value);
    }

    void broadcast (Broadcast::buttonPress& args) {
        printf("buttonPress! %" PRId32 " %" PRId32 "\n", args.button, args.mask);
    }

private:
    std::function<void(const BufferType&)> mPostFunc;
};

#endif
