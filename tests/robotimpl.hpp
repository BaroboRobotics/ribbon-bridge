#ifndef RPC_TESTS_ROBOTIMPL_HPP
#define RPC_TESTS_ROBOTIMPL_HPP

#include "rpc/componenttraits.hpp"

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

#include <future>

template <class Out>
struct StupidFutureTemplate {
public:
    StupidFutureTemplate (std::future<Out>&& future, rpc::Buffer<256> buffer)
            : mFuture(std::move(future))
            , mBuffer(buffer) { }

    StupidFutureTemplate (rpc::Error error)
            : mError(error) { }

    std::future<Out>& future () {
        return mFuture;
    }

    rpc::Buffer<256>& buffer () {
        return mBuffer;
    }

    rpc::Error error () {
        return mError;
    }

private:
    std::future<Out> mFuture;
    rpc::Buffer<256> mBuffer;
    rpc::Error mError = rpc::Error::NO_ERROR;
};

class RobotService;
class RobotProxy;

class RobotService : public rpc::Service<RobotService, com::barobo::Robot, StupidFutureTemplate> {
public:
    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using RobotMethodIn = rpc::MethodIn<com::barobo::Robot>;
    using RobotMethodOut = rpc::MethodOut<com::barobo::Robot>;
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
    RobotMethodOut::move on (RobotMethodIn::move& args) {
        printf("%f %f %f\n", double(args.desiredAngle1),
                double(args.desiredAngle2), double(args.desiredAngle3));
        RobotMethodOut::move output;
        output.has_out = true;
        output.out.funFactor = 1.23;
        return output;
    }

    /* More methods ... */

private:
    RobotAttribute::motorPower motorPower;
};

class RobotProxy : public rpc::Proxy<RobotProxy, com::barobo::Robot, StupidFutureTemplate> {
public:
    rpc::Error fulfillWithError (uint32_t requestId, rpc::Error error) {
        printf("fulfillWithError\n");
        return error;
    }

    template <class Out>
    rpc::Error fulfillWithOutput (uint32_t requestId, Out& out) {
        printf("fulfillWithOutput\n");
        return rpc::Error::NO_ERROR;
    }

    template <class C>
    StupidFutureTemplate<C> finalize (uint32_t requestId, uint32_t componentId, rpc::Error error) {
        return { error };
    }

    template <class C>
    StupidFutureTemplate<C> finalize (uint32_t requestId, uint32_t componentId, BufferType& buffer) {
        return { std::future<C>(), buffer };
    }

    using RobotAttribute = rpc::Attribute<com::barobo::Robot>;
    using RobotBroadcast = rpc::Broadcast<com::barobo::Robot>;

    void broadcast (RobotAttribute::motorPower& args) {
        printf("motorPower! %" PRId32 "\n", args.value);
    }

    void broadcast (RobotBroadcast::buttonPress& args) {
        printf("buttonPress! %" PRId32 " %" PRId32 "\n", args.button, args.mask);
    }

private:
};

#endif
