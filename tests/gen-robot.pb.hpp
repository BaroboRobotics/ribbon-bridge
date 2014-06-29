#ifndef COM_BAROBO_ROBOT_INTERFACE
#define COM_BAROBO_ROBOT_INTERFACE

/* GENERATED CODE */

#include "rpc/service.hpp"
#include "rpc/proxy.hpp"
#include "rpc/hash.hpp"
#include "robot.pb.h"

namespace com {
namespace barobo {

/* Forward declaration of interface */
template <class Derived>
class Robot;

} // namespace barobo
} // namespace com

namespace rpc {

template <>
struct Attribute<com::barobo::Robot> {
    using motorPower = com_barobo_Robot_motorPower;
};

template <>
struct IsAttribute<Attribute<com::barobo::Robot>::motorPower> {
    constexpr static const bool value = true;
};

template <>
struct Method<com::barobo::Robot> {
    using move = com_barobo_Robot_move;
};

template <>
struct IsMethod<Method<com::barobo::Robot>::move> {
    constexpr static const bool value = true;
};

template <>
struct Broadcast<com::barobo::Robot> {
    using buttonPress = com_barobo_Robot_buttonPress;
};

template <>
struct IsBroadcast<Broadcast<com::barobo::Robot>::buttonPress> {
    constexpr static const bool value = true;
};

template <>
union ComponentUnion<com::barobo::Robot> {
    typename Attribute<com::barobo::Robot>::motorPower motorPower;
    typename Method<com::barobo::Robot>::move move;
    typename Broadcast<com::barobo::Robot>::buttonPress buttonPress;
};

template <>
void decodePayload (ComponentUnion<com::barobo::Robot>& args, com_barobo_rpc_ToObject& toObject);

template <>
struct ComponentId<com::barobo::Robot> {
    enum : uint32_t {
        motorPower = hash("motorPower"),
        move = hash("move"),
        buttonPress = hash("buttonPress")
    };
};

template <>
constexpr uint32_t componentId (Method<com::barobo::Robot>::move) {
    return ComponentId<com::barobo::Robot>::move;
}

template <class T>
void fire (T& service,
        ComponentUnion<com::barobo::Robot>& argument,
        com_barobo_rpc_ToObject& toObject) {
    /* TODO: static_assert that T implements com::barobo::Robot */
    switch (toObject.componentId) {
        case ComponentId<com::barobo::Robot>::motorPower:
            if (com_barobo_rpc_ToObject_Type_GET == toObject.type) {
                service.on_(argument.motorPower, Get());
            }
            else if (com_barobo_rpc_ToObject_Type_SET == toObject.type) {
                service.on_(argument.motorPower, Set());
            }
            else {
                // yo wtf
                assert(false);
            }
            break;
        case ComponentId<com::barobo::Robot>::move:
            service.on_(argument.move);
            break;
        default:
            assert(false);
            break;
    }
}

} // namespace rpc

namespace com {
namespace barobo {

template <class Derived>
struct Robot {
    /* Attribute motorPower getter */
    typename rpc::Attribute<Robot>::motorPower motorPower () const {
        rpc::Attribute<Robot>::motorPower args;
        static_cast<Derived&>(*this).on_(args, rpc::Get());
        return args;
    }

    /* Attribute motorPower setter - only present if attribute is not readonly */
    void motorPower (typename rpc::Attribute<Robot>::motorPower args) {
        static_cast<Derived&>(*this).on_(args, rpc::Set());
        /* Attribute broadcast - only present if attribute is not
         * noSubscriptions */
        static_cast<Derived&>(*this).on_(args, rpc::Notify());
    }

    /* Method move */
    void move (float desiredAngle1, float desiredAngle2, float desiredAngle3) {
        using Method = typename rpc::Method<Robot>::move;
        Method args {
            { desiredAngle1, desiredAngle2, desiredAngle3 }
        };
        static_cast<Derived&>(*this).on_(args);
    }

    /* Broadcast buttonPress */
    void buttonPress (uint32_t button, uint32_t mask) {
        rpc::Broadcast<Robot>::buttonPress args {
            button, mask
        };
        static_cast<Derived&>(*this).on_(args);
    }
};

} // namespace barobo
} // namespace com

#endif
