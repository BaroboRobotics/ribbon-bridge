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
void decodePayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        com_barobo_rpc_Request_Component_Invocation& invocation);

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

template <>
struct Invoker<com::barobo::Robot> {
    template <class T>
    static void invoke (T& service,
            ComponentUnion<com::barobo::Robot>& argument,
            uint32_t componentId,
            com_barobo_rpc_Request_Component_Invocation& invocation) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        switch (componentId) {
            case ComponentId<com::barobo::Robot>::motorPower:
                if (invocation.payload.size) {
                    service.on(argument.motorPower, Set());
                }
                else {
                    service.on(argument.motorPower, Get());
                }
                break;
            case ComponentId<com::barobo::Robot>::move:
                service.on(argument.move);
                break;
            default:
                assert(false);
                break;
        }
    }
};

} // namespace rpc

namespace com {
namespace barobo {

template <class Derived>
struct Robot;

template <template <class, template <class> class> class R, class T, template <class> class I>
struct Robot<R<T, I>> {
    using Derived = R<T, I>;

    template <class C>
    using ReturnType = 
        typename rpc::ImplementationTraits<T>::template ReturnType<C>;

    /* Attribute motorPower getter */

    ReturnType<typename rpc::Attribute<Robot>::motorPower> motorPower () const {
        rpc::Attribute<Robot>::motorPower args;
        return ReturnType<typename rpc::Attribute<Robot>::motorPower>(reinterpret_cast<const Derived*>(this)->on(args, rpc::Get()));
    }

    /* Attribute motorPower setter - only present if attribute is not readonly */
    ReturnType<void> motorPower (typename rpc::Attribute<Robot>::motorPower args) {
        reinterpret_cast<Derived*>(this)->on(args, rpc::Set());
        /* Attribute broadcast - only present if attribute is not
         * noSubscriptions */
        return ReturnType<void>(reinterpret_cast<Derived*>(this)->on(args, rpc::Notify()));
    }

    /* Method move */
    ReturnType<typename rpc::Method<Robot>::move> move (float desiredAngle1, float desiredAngle2, float desiredAngle3) {
        rpc::Method<Robot>::move args {
            { desiredAngle1, desiredAngle2, desiredAngle3 }
        };
        return ReturnType<typename rpc::Method<Robot>::move>(reinterpret_cast<Derived*>(this)->on(args));
    }

    /* Broadcast buttonPress */
    void buttonPress (uint32_t button, uint32_t mask) {
        rpc::Broadcast<Robot>::buttonPress args {
            button, mask
        };
        reinterpret_cast<Derived*>(this)->on(args);
    }
};

} // namespace barobo
} // namespace com

#endif
