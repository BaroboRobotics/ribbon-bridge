#ifndef RPC_TESTS_ROBOTIMPL_HPP
#define RPC_TESTS_ROBOTIMPL_HPP

#include "rpc/componenttraits.hpp"

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

#include <boost/any.hpp>
#include <boost/unordered_map.hpp>
#include <future>
#include <utility>
#include <tuple>
#include <boost/variant.hpp>

template <class Out>
struct StupidFutureTemplate {
public:
    StupidFutureTemplate (std::future<Out>&& future, rpc::Buffer<256> buffer)
            : mFuture(std::move(future))
            , mBuffer(buffer) { }

    StupidFutureTemplate (rpc::Status status)
            : mStatus(status) { }

    std::future<Out>& future () {
        return mFuture;
    }

    rpc::Buffer<256>& buffer () {
        return mBuffer;
    }

    rpc::Status status () {
        return mStatus;
    }

private:
    std::future<Out> mFuture;
    rpc::Buffer<256> mBuffer;
    rpc::Status mStatus = rpc::Status::OK;
};

class RobotService;
class RobotProxy;

class RobotService : public rpc::Service<RobotService, com::barobo::Robot> {
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
    RobotMethodOut::move fire (RobotMethodIn::move& args) {
        printf("%f %f %f\n", double(args.desiredAngle1),
                double(args.desiredAngle2), double(args.desiredAngle3));
        RobotMethodOut::move result;
        result.has_out = true;
        result.out.funFactor = 1.23;
        return result;
    }

    /* More methods ... */

private:
    RobotAttribute::motorPower motorPower;
};

class RobotProxy : public rpc::Proxy<RobotProxy, com::barobo::Robot, StupidFutureTemplate> {
    template <class... Ts>
    using MakePromiseVariant = boost::variant<std::promise<Ts>...>;
    using PromiseVariant = typename rpc::ComponentOutVariadic<com::barobo::Robot, MakePromiseVariant>::type;

public:
    rpc::Status fulfillWithStatus (uint32_t requestId, rpc::Status status) {
        printf("fulfillWithStatus\n");
        return status;
    }

    template <class Out>
    rpc::Status fulfillWithResult (uint32_t requestId, Out& out) {
        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            // FIXME better error
            return rpc::Status::INCONSISTENT_REPLY;
        }
        auto promisePtr = boost::get<std::promise<Out>>(&iter->second);
        if (!promisePtr) {
            // FIXME better error
            return rpc::Status::INCONSISTENT_REPLY;
        }
        promisePtr->set_value(out);
        mPromises.erase(iter);
        return rpc::Status::OK;
    }

    template <class C>
    StupidFutureTemplate<C> finalize (uint32_t requestId, rpc::Status status) {
        return { status };
    }

    template <class C>
    StupidFutureTemplate<C> finalize (uint32_t requestId, BufferType& buffer) {
        typename decltype(mPromises)::iterator iter;
        bool success;
        std::tie(iter, success) = mPromises.emplace(std::piecewise_construct,
                std::make_tuple(requestId),
                std::make_tuple(std::promise<C>()));
        if (!success) {
            // this will break the existing promise
            mPromises.erase(iter);
            std::tie(iter, success) = mPromises.emplace(std::piecewise_construct,
                    std::make_tuple(requestId),
                    std::make_tuple(std::promise<C>()));
            assert(success);
        }

        auto promisePtr = boost::get<std::promise<C>>(&iter->second);
        assert(promisePtr);
        return { promisePtr->get_future(), buffer };
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
    boost::unordered_map
        < uint32_t
        , PromiseVariant
        > mPromises;
};

#endif
