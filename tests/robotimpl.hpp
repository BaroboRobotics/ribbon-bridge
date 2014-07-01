#ifndef RPC_TESTS_ROBOTIMPL_HPP
#define RPC_TESTS_ROBOTIMPL_HPP

#include "rpc/potqueue.hpp"

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

template <class T>
struct Future { };

class RobotService;
class RobotProxy;

namespace rpc {

template <>
struct ImplementationTraits<RobotService> {
    template <class>
    using ReturnType = void;
};

template <>
struct ImplementationTraits<RobotProxy> {
    template <class C>
    using ReturnType = ::Future<C>;
};

}

class RobotService : public rpc::Service<RobotService, com::barobo::Robot> {
public:
    void post (BufferType buffer) {
        mOutputQueue.push(buffer);
    }

    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using RobotMethod = rpc::Method<com::barobo::Robot>;
    using RobotAttribute = rpc::Attribute<com::barobo::Robot>;

    void on (RobotAttribute::motorPower& args, rpc::Set) {
        motorPower = args;
    }

    void on (RobotAttribute::motorPower& args, rpc::Get) {
        args = motorPower;
    }

    /* Methods are implemented by overloading the "on_" member function, and
     * taking the interface method structure (containing input and output
     * parameter structures, and an error field) as the single reference
     * parameter. */
    void on (RobotMethod::move& args) {
        auto& in = args.in;
        printf("%f %f %f\n", double(in.desiredAngle1),
                double(in.desiredAngle2), double(in.desiredAngle3));
    }

    /* More methods ... */

private:
    RobotAttribute::motorPower motorPower;

    rpc::PotQueue<BufferType, 2> mOutputQueue;
};

class RobotProxy : public rpc::Proxy<RobotProxy, com::barobo::Robot> {
public:
    void post (BufferType buffer) {
        mOutputQueue.push(buffer);
    }

    template <class C>
    using ReturnType = typename rpc::ImplementationTraits<RobotProxy>::template ReturnType<C>;

    template <class Component>
    ReturnType<Component> promise () {
        printf("encoding error\n");
        //throw "encoding error";
        return { };
    }

    template <class Component>
    ReturnType<Component> promise (uint32_t requestId) {
        printf("okay, I promise %" PRId32 "\n", requestId);
        return { };
    }

    using RobotBroadcast = rpc::Broadcast<com::barobo::Robot>;

    void on (RobotBroadcast::buttonPress& args) {
        printf("buttonPress! %" PRId32 " %" PRId32 "\n", args.button, args.mask);
    }

    bool tryPop (BufferType& buffer) {
        if (mOutputQueue.empty()) {
            return false;
        }
        buffer = mOutputQueue.front();
        mOutputQueue.pop();
        return true;
    }

private:
    rpc::PotQueue<BufferType, 2> mOutputQueue;
};

#endif
