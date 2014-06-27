#ifndef RPC_TESTS_ROBOTIMPL_HPP
#define RPC_TESTS_ROBOTIMPL_HPP

/* Implementation of the com::barobo::Robot interface. */

/* You first need to include the generated interface code. */
#include "gen-robot.pb.hpp"

class RobotImpl : public rpc::Object<RobotImpl, com::barobo::Robot> {
public:
    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using RobotMethod = rpc::Method<com::barobo::Robot>;
    using RobotAttribute = rpc::Attribute<com::barobo::Robot>;

    void on_ (RobotAttribute::motorPower& args, rpc::Set) {
        motorPower = args;
        rpc::Object<RobotImpl, com::barobo::Robot>::broadcast_(args);
    }

    void on_ (RobotAttribute::motorPower& args, rpc::Get) {
        args = motorPower;
    }

    /* Methods are implemented by overloading the "on_" member function, and
     * taking the interface method structure (containing input and output
     * parameter structures, and an error field) as the single reference
     * parameter. */
    void on_ (RobotMethod::move& args) {
        auto& in = args.in;
        printf("%f %f %f\n", double(in.desiredAngle1),
                double(in.desiredAngle2), double(in.desiredAngle3));
    }

    /* More methods ... */

private:
    RobotAttribute::motorPower motorPower;
};

#endif
