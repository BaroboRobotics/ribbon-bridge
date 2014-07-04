#ifndef RPC_TESTS_ROBOTIMPL_HPP
#define RPC_TESTS_ROBOTIMPL_HPP

#include "rpc/componenttraits.hpp"

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

#include <future>

class RobotService;
class RobotProxy;

class RobotService : public rpc::Service<RobotService, com::barobo::Robot> {
public:
    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using RobotMethod = rpc::Method<com::barobo::Robot>;
    using RobotAttribute = rpc::Attribute<com::barobo::Robot>;

    RobotAttribute::motorPower get (RobotAttribute::motorPower) {
        return motorPower;
    }

    void set (RobotAttribute::motorPower args) {
        motorPower = args;
    }

    /* Methods are implemented by overloading the "on_" member function, and
     * taking the interface method structure (containing input and output
     * parameter structures, and an error field) as the single reference
     * parameter. */
    ResultOf<RobotMethod::move> on (RobotMethod::move& args) {
        printf("%f %f %f\n", double(args.desiredAngle1),
                double(args.desiredAngle2), double(args.desiredAngle3));
    }

    /* More methods ... */

private:
    RobotAttribute::motorPower motorPower;
};

template <class Method>
struct StupidFutureTemplate {
    static_assert(rpc::IsMethod<Method>::value, "StupidFutureTemplate can only be used with methods");
public:
    using Future = std::future<typename rpc::ResultOf<Method>::type>;

    StupidFutureTemplate (Future&& future, rpc::Buffer<256>&& buffer)
            : mFuture(future)
            , mBuffer(buffer) { }

    StupidFutureTemplate (rpc::Error error)
            : mError(error) { }

    Future& future () {
        return mFuture;
    }

    rpc::Buffer<256>& buffer () {
        return mBuffer;
    }

    rpc::Error error () {
        return mError;
    }

private:
    Future mFuture;
    rpc::Buffer<256> mBuffer;
    rpc::Error mError = rpc::Error::NO_ERROR;
};

class RobotProxy : public rpc::Proxy<RobotProxy, com::barobo::Robot, StupidFutureTemplate> {
public:
    using RobotBroadcast = rpc::Broadcast<com::barobo::Robot>;

    void broadcast (RobotBroadcast::buttonPress& args) {
        printf("buttonPress! %" PRId32 " %" PRId32 "\n", args.button, args.mask);
    }

private:
};

#endif
