#ifndef COM_BAROBO_ROBOT_INTERFACE
#define COM_BAROBO_ROBOT_INTERFACE

/* GENERATED CODE */

#include "rpc/checkversion.hpp"
#include "rpc/componenttraits.hpp"
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
struct MethodIn<com::barobo::Robot> {
    using move = com_barobo_Robot_move_In;
};

template <>
struct MethodResult<com::barobo::Robot> {
    using move = com_barobo_Robot_move_Result;
};

template <>
struct ResultOf<com_barobo_Robot_move_In> {
    using type = com_barobo_Robot_move_Result;
};

template <>
struct IsMethod<MethodIn<com::barobo::Robot>::move> {
    constexpr static const bool value = true;
};

template <>
struct IsMethod<MethodResult<com::barobo::Robot>::move> {
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
union ComponentInUnion<com::barobo::Robot> {
    typename Attribute<com::barobo::Robot>::motorPower motorPower;
    typename MethodIn<com::barobo::Robot>::move move;
};

template <>
union ComponentResultUnion<com::barobo::Robot> {
    typename Attribute<com::barobo::Robot>::motorPower motorPower;
    typename MethodResult<com::barobo::Robot>::move move;
    typename Broadcast<com::barobo::Robot>::buttonPress buttonPress;
};

template <template <class...> class F>
struct PromiseVariadic<com::barobo::Robot, F> {
    /* Apply all the promisables (attributes and method results, but not
     * broadcasts) to F. Note that we need to include void here to cover
     * attribute set commands and subscribes/unsubscribes. */
    using type = F
        < void
        , typename Attribute<com::barobo::Robot>::motorPower
        , typename MethodResult<com::barobo::Robot>::move
        >;
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
constexpr uint32_t componentId (com_barobo_Robot_move_Result) {
    return ComponentId<com::barobo::Robot>::move;
}

template <>
constexpr uint32_t componentId (com_barobo_Robot_buttonPress) {
    return ComponentId<com::barobo::Robot>::buttonPress;
}

template <>
bool isMethod<com::barobo::Robot> (uint32_t id);

template <>
class Subscriptions<com::barobo::Robot> {
public:
    using Id = ComponentId<com::barobo::Robot>;

    Status activate (uint32_t id) {
        switch (id) {
            case Id::motorPower:
                motorPower = true;
                return Status::OK;
            case Id::buttonPress:
                buttonPress = true;
                return Status::OK;
            default:
                return isMethod<com::barobo::Robot>(id) ?
                           Status::ILLEGAL_OPERATION :
                           Status::NO_SUCH_COMPONENT;
        }
    }

    Status deactivate (uint32_t id) {
        switch (id) {
            case Id::motorPower:
                motorPower = false;
                return Status::OK;
            case Id::buttonPress:
                buttonPress = false;
                return Status::OK;
            default:
                return isMethod<com::barobo::Robot>(id) ?
                           Status::ILLEGAL_OPERATION :
                           Status::NO_SUCH_COMPONENT;
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
    static Status invoke (T& service,
            ComponentInUnion<com::barobo::Robot>& argument,
            uint32_t componentId,
            com_barobo_rpc_Reply_Result_payload_t& payload) {
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
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct SetInvoker<com::barobo::Robot> {
    template <class T>
    static Status invoke (T& service,
            ComponentInUnion<com::barobo::Robot>& argument,
            uint32_t componentId) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of non-read-only attributes
            case Id::motorPower:
                service.set(argument.motorPower);
                return Status::OK;
            default:
                if (isAttribute<com::barobo::Robot>(componentId)) {
                    return Status::READ_ONLY;
                }
                return isComponent<com::barobo::Robot>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct FireInvoker<com::barobo::Robot> {
    template <class T>
    static Status invoke (T& service,
            ComponentInUnion<com::barobo::Robot>& argument,
            uint32_t componentId,
            com_barobo_rpc_Reply_Result_payload_t& payload) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of methods
            case Id::move: {
                auto val = service.fire(argument.move);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            default:
                return isComponent<com::barobo::Robot>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct BroadcastInvoker<com::barobo::Robot> {
    template <class T>
    static Status invoke (T& proxy,
            ComponentResultUnion<com::barobo::Robot>& argument,
            uint32_t componentId) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of subscribable attributes
            case Id::motorPower:
                proxy.broadcast(argument.motorPower);
                return Status::OK;
            // list of broadcasts
            case Id::buttonPress:
                proxy.broadcast(argument.buttonPress);
                return Status::OK;
            default:
                if (isAttribute<com::barobo::Robot>(componentId)) {
                    return Status::NO_SUBSCRIPTIONS;
                }
                return isComponent<com::barobo::Robot>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct FulfillInvoker<com::barobo::Robot> {
    template <class T>
    static Status invoke (T& proxy,
            ComponentResultUnion<com::barobo::Robot>& argument,
            uint32_t componentId,
            uint32_t requestId) {
        /* TODO: static_assert that T implements com::barobo::Robot */
        using Id = ComponentId<com::barobo::Robot>;
        switch (componentId) {
            // list of attributes
            case Id::motorPower:
                return proxy.fulfill(requestId, argument.motorPower);
            // list of methods
            case Id::move:
                return proxy.fulfill(requestId, argument.move);
            default:
                return isBroadcast<com::barobo::Robot>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

} // namespace rpc

#endif
