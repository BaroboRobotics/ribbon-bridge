#ifndef COM_BAROBO_ROBOT_INTERFACE
#define COM_BAROBO_ROBOT_INTERFACE

/* GENERATED CODE */

#include "rpc/variant.hpp"
#include "rpc/service.hpp"
#include "rpc/proxy.hpp"
#include "rpc/hash.hpp"
#include "robot.pb.h"

namespace com {
namespace barobo {
struct Robot;
} // namespace barobo
} // namespace com

namespace rpc {

template <>
struct Version<com::barobo::Robot> {
    constexpr static const uint32_t major = 0;
    constexpr static const uint32_t minor = 0;
    constexpr static const uint32_t patch = 0;
};

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
    using move = com_barobo_Robot_move_In;
};

template <>
struct ResultOf<com_barobo_Robot_move_In> {
    using type = Variant<com_barobo_Robot_move_Out, com_barobo_Robot_move_Error>;
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

/* COMPONENT IDs */

template <>
struct ComponentId<com::barobo::Robot> {
    enum : uint32_t {
        motorPower = hash("motorPower"),
        move = hash("move"),
        buttonPress = hash("buttonPress")
    };
};

template <>
constexpr uint32_t componentId (com_barobo_Robot_motorPower) {
    return ComponentId<com::barobo::Robot>::motorPower;
}

template <>
constexpr uint32_t componentId (com_barobo_Robot_move) {
    return ComponentId<com::barobo::Robot>::move;
}

template <>
constexpr uint32_t componentId (com_barobo_Robot_move_In) {
    return ComponentId<com::barobo::Robot>::move;
}

template <>
constexpr uint32_t componentId (com_barobo_Robot_move_Out) {
    return ComponentId<com::barobo::Robot>::move;
}

template <>
constexpr uint32_t componentId (com_barobo_Robot_move_Error) {
    return ComponentId<com::barobo::Robot>::move;
}

template <>
constexpr uint32_t componentId (com_barobo_Robot_buttonPress) {
    return ComponentId<com::barobo::Robot>::buttonPress;
}

template <>
class Subscriptions<com::barobo::Robot> {
public:
    using Id = ComponentId<com::barobo::Robot>;

    Error activate (uint32_t id) {
        switch (id) {
            case Id::motorPower:
                motorPower = true;
                return Error::NO_ERROR;
            case Id::buttonPress:
                buttonPress = true;
                return Error::NO_ERROR;
            default:
                return isMethod<com::barobo::Robot>(id) ?
                           Error::ILLEGAL_OPERATION :
                           Error::NO_SUCH_COMPONENT;
        }
    }

    Error deactivate (uint32_t id) {
        switch (id) {
            case Id::motorPower:
                motorPower = false;
                return Error::NO_ERROR;
            case Id::buttonPress:
                buttonPress = false;
                return Error::NO_ERROR;
            default:
                return isMethod<com::barobo::Robot>(id) ?
                           Error::ILLEGAL_OPERATION :
                           Error::NO_SUCH_COMPONENT;
        }
    }

    bool isActive (uint32_t id) {
        switch (id) {
            case Id::motorPower:
                return motorPower;
            case Id::buttonPress:
                return buttonPress;
            default:
                return false;
        }
    }

    void reset () {
        motorPower = false;
        buttonPress = false;
    }

private:
    bool motorPower = false;
    bool buttonPress = false;
};

template <>
struct GetInvoker<com::barobo::Robot> {
    template <class T>
    static Error invoke (T& service,
            ComponentUnion<com::barobo::Robot>& argument,
            uint32_t componentId,
            com_barobo_rpc_Reply_Output_payload_t& payload) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of attributes
            case Id::motorPower: {
                auto val = service.get(argument.motorPower);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            default:
                return isComponent<com::barobo::Robot>(componentId) ?
                    Error::ILLEGAL_OPERATION :
                    Error::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct SetInvoker<com::barobo::Robot> {
    template <class T>
    static Error invoke (T& service,
            ComponentUnion<com::barobo::Robot>& argument,
            uint32_t componentId) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of non-read-only attributes
            case Id::motorPower:
                service.set(argument.motorPower);
                return Error::NO_ERROR;
            default:
                if (isAttribute<com::barobo::Robot>(componentId)) {
                    return Error::READ_ONLY;
                }
                return isComponent<com::barobo::Robot>(componentId) ?
                    Error::ILLEGAL_OPERATION :
                    Error::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct FireInvoker<com::barobo::Robot> {
    template <class T>
    static Error invoke (T& service,
            ComponentUnion<com::barobo::Robot>& argument,
            uint32_t componentId,
            com_barobo_rpc_Reply_Output_payload_t& payload,
            bool& isExceptional) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of methods
            case Id::move: {
                auto val = service.on(argument.move);
                payload.size = sizeof(payload.bytes);
                if (val.isOut()) {
                    return encodeProtobuf(&val.getOut(), pbFields(val.getOut()), payload.bytes, payload.size, payload.size);
                }
                else {
                    isExceptional = true;
                    return encodeProtobuf(&val.getError(), pbFields(val.getError()), payload.bytes, payload.size, payload.size);
                }
            }
            default:
                return isComponent<com::barobo::Robot>(componentId) ?
                    Error::ILLEGAL_OPERATION :
                    Error::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct BroadcastInvoker<com::barobo::Robot> {
    template <class T>
    static Error invoke (T& service,
            ComponentUnion<com::barobo::Robot>& argument,
            uint32_t componentId) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of subscribable attributes
            case Id::motorPower:
                service.broadcast(argument.motorPower);
                return Error::NO_ERROR;
            // list of broadcasts
            case Id::buttonPress:
                service.broadcast(argument.buttonPress);
                return Error::NO_ERROR;
            default:
                if (isAttribute<com::barobo::Robot>(componentId)) {
                    return Error::NO_SUBSCRIPTIONS;
                }
                return isComponent<com::barobo::Robot>(componentId) ?
                    Error::ILLEGAL_OPERATION :
                    Error::NO_SUCH_COMPONENT;
        }
    }
};

#if 0
template <class T, class Interface>
Error invokeFulfillWithError (T& service,
        Error argument,
        uint32_t requestId) {
    return FulfillWithErrorInvoker<Interface>::invoke(service, argument, requestId);
}

template <class T, class Interface>
Error invokeFulfillWithOutputError (T& service,
        ComponentUnion<Interface>& argument,
        uint32_t requestId) {
    return FulfillWithErrorInvoker<Interface>::invoke(service, argument, requestId);
}

template <class T, class Interface>
Error invokeFulfillWithOutputOut (T& service,
        ComponentUnion<Interface>& argument,
        uint32_t requestId) {
    return FulfillWithErrorInvoker<Interface>::invoke(service, argument, requestId);
}
#endif

} // namespace rpc

#endif
