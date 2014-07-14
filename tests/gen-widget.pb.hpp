#ifndef barobo_Widget_INTERFACE
#define barobo_Widget_INTERFACE

/* GENERATED CODE */

#include "rpc/checkversion.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/hash.hpp"
#include "widget.pb.h"

namespace barobo {
struct Widget;
} // namespace barobo

namespace rpc {

template <>
bool isAttribute<barobo::Widget> (uint32_t id);
template <>
bool isMethod<barobo::Widget> (uint32_t id);
template <>
bool isBroadcast<barobo::Widget> (uint32_t id);

//////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE TRAITS

template <>
struct Attribute<barobo::Widget> {
    using attribute = barobo_Widget_attribute;
    using readonlyAttribute = barobo_Widget_readonlyAttribute;
    using noSubscriptionsAttribute = barobo_Widget_noSubscriptionsAttribute;
    using readonlyNoSubscriptionsAttribute = barobo_Widget_readonlyNoSubscriptionsAttribute;
};

template <>
struct IsAttribute<Attribute<barobo::Widget>::attribute> {
    constexpr static const bool value = true;
};
template <>
struct IsAttribute<Attribute<barobo::Widget>::readonlyAttribute> {
    constexpr static const bool value = true;
};
template <>
struct IsAttribute<Attribute<barobo::Widget>::noSubscriptionsAttribute> {
    constexpr static const bool value = true;
};
template <>
struct IsAttribute<Attribute<barobo::Widget>::readonlyNoSubscriptionsAttribute> {
    constexpr static const bool value = true;
};

template <>
struct IsSettableAttribute<Attribute<barobo::Widget>::attribute> {
    constexpr static const bool value = true;
};
template <>
struct IsSettableAttribute<Attribute<barobo::Widget>::noSubscriptionsAttribute> {
    constexpr static const bool value = true;
};
template <>
struct IsSubscribableAttribute<Attribute<barobo::Widget>::attribute> {
    constexpr static const bool value = true;
};
template <>
struct IsSubscribableAttribute<Attribute<barobo::Widget>::readonlyAttribute> {
    constexpr static const bool value = true;
};

//////////////////////////////////////////////////////////////////////////////
// METHOD TRAITS

template <>
struct MethodIn<barobo::Widget> {
    using nullaryNoResult = barobo_Widget_nullaryNoResult_In;
    using nullaryWithResultOut = barobo_Widget_nullaryWithResultOut_In;
    using nullaryWithResultError = barobo_Widget_nullaryWithResultError_In;
    using nullaryWithResult = barobo_Widget_nullaryWithResult_In;
    using unaryNoResult = barobo_Widget_unaryNoResult_In;
    using unaryWithResultOut = barobo_Widget_unaryWithResultOut_In;
    using unaryWithResultError = barobo_Widget_unaryWithResultError_In;
    using unaryWithResult = barobo_Widget_unaryWithResult_In;
};

template <>
struct MethodResult<barobo::Widget> {
    using nullaryNoResult = barobo_Widget_nullaryNoResult_Result;
    using nullaryWithResultOut = barobo_Widget_nullaryWithResultOut_Result;
    using nullaryWithResultError = barobo_Widget_nullaryWithResultError_Result;
    using nullaryWithResult = barobo_Widget_nullaryWithResult_Result;
    using unaryNoResult = barobo_Widget_unaryNoResult_Result;
    using unaryWithResultOut = barobo_Widget_unaryWithResultOut_Result;
    using unaryWithResultError = barobo_Widget_unaryWithResultError_Result;
    using unaryWithResult = barobo_Widget_unaryWithResult_Result;
};

template <>
struct ResultOf<MethodIn<barobo::Widget>::nullaryNoResult> {
    using type = MethodResult<barobo::Widget>::nullaryNoResult;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::nullaryWithResultOut> {
    using type = MethodResult<barobo::Widget>::nullaryWithResultOut;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::nullaryWithResultError> {
    using type = MethodResult<barobo::Widget>::nullaryWithResultError;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::nullaryWithResult> {
    using type = MethodResult<barobo::Widget>::nullaryWithResult;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::unaryNoResult> {
    using type = MethodResult<barobo::Widget>::unaryNoResult;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::unaryWithResultOut> {
    using type = MethodResult<barobo::Widget>::unaryWithResultOut;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::unaryWithResultError> {
    using type = MethodResult<barobo::Widget>::unaryWithResultError;
};
template <>
struct ResultOf<MethodIn<barobo::Widget>::unaryWithResult> {
    using type = MethodResult<barobo::Widget>::unaryWithResult;
};

template <>
struct IsMethod<MethodIn<barobo::Widget>::nullaryNoResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::nullaryWithResultOut> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::nullaryWithResultError> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::nullaryWithResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::unaryNoResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::unaryWithResultOut> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::unaryWithResultError> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodIn<barobo::Widget>::unaryWithResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::nullaryNoResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::nullaryWithResultOut> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::nullaryWithResultError> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::nullaryWithResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::unaryNoResult> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::unaryWithResultOut> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::unaryWithResultError> {
    constexpr static const bool value = true;
};
template <>
struct IsMethod<MethodResult<barobo::Widget>::unaryWithResult> {
    constexpr static const bool value = true;
};

//////////////////////////////////////////////////////////////////////////////
// BROADCAST TRAITS

template <>
struct Broadcast<barobo::Widget> {
    using broadcast = barobo_Widget_broadcast;
};

template <>
struct IsBroadcast<Broadcast<barobo::Widget>::broadcast> {
    constexpr static const bool value = true;
};

//////////////////////////////////////////////////////////////////////////////
// INTERFACE TRAITS

template <>
struct Version<barobo::Widget> {
    constexpr static const uint32_t major = 0;
    constexpr static const uint32_t minor = 0;
    constexpr static const uint32_t patch = 0;
};

template <>
union ComponentInUnion<barobo::Widget> {
    typename Attribute<barobo::Widget>::attribute attribute;
    //typename Attribute<barobo::Widget>::readonlyAttribute readonlyAttribute;
    typename Attribute<barobo::Widget>::noSubscriptionsAttribute noSubscriptionsAttribute;
    //typename Attribute<barobo::Widget>::readonlyNoSubscriptionsAttribute readonlyNoSubscriptionsAttribute;
    typename MethodIn<barobo::Widget>::nullaryNoResult nullaryNoResult;
    typename MethodIn<barobo::Widget>::nullaryWithResultOut nullaryWithResultOut;
    typename MethodIn<barobo::Widget>::nullaryWithResultError nullaryWithResultError;
    typename MethodIn<barobo::Widget>::nullaryWithResult nullaryWithResult;
    typename MethodIn<barobo::Widget>::unaryNoResult unaryNoResult;
    typename MethodIn<barobo::Widget>::unaryWithResultOut unaryWithResultOut;
    typename MethodIn<barobo::Widget>::unaryWithResultError unaryWithResultError;
    typename MethodIn<barobo::Widget>::unaryWithResult unaryWithResult;
};

template <>
union ComponentResultUnion<barobo::Widget> {
    typename Attribute<barobo::Widget>::attribute attribute;
    typename Attribute<barobo::Widget>::readonlyAttribute readonlyAttribute;
    typename Attribute<barobo::Widget>::noSubscriptionsAttribute noSubscriptionsAttribute;
    typename Attribute<barobo::Widget>::readonlyNoSubscriptionsAttribute readonlyNoSubscriptionsAttribute;
    typename MethodResult<barobo::Widget>::nullaryNoResult nullaryNoResult;
    typename MethodResult<barobo::Widget>::nullaryWithResultOut nullaryWithResultOut;
    typename MethodResult<barobo::Widget>::nullaryWithResultError nullaryWithResultError;
    typename MethodResult<barobo::Widget>::nullaryWithResult nullaryWithResult;
    typename MethodResult<barobo::Widget>::unaryNoResult unaryNoResult;
    typename MethodResult<barobo::Widget>::unaryWithResultOut unaryWithResultOut;
    typename MethodResult<barobo::Widget>::unaryWithResultError unaryWithResultError;
    typename MethodResult<barobo::Widget>::unaryWithResult unaryWithResult;
    typename Broadcast<barobo::Widget>::broadcast broadcast;
};

template <template <class...> class F>
struct PromiseVariadic<barobo::Widget, F> {
    /* Apply all the promisables (attributes and method results, but not
     * broadcasts) to F. Note that we need to include void here to cover
     * attribute set commands and subscribes/unsubscribes. */
    using type = F
        < void
        , typename Attribute<barobo::Widget>::attribute
        , typename Attribute<barobo::Widget>::readonlyAttribute
        , typename Attribute<barobo::Widget>::noSubscriptionsAttribute
        , typename Attribute<barobo::Widget>::readonlyNoSubscriptionsAttribute
        , typename MethodResult<barobo::Widget>::nullaryNoResult
        , typename MethodResult<barobo::Widget>::nullaryWithResultOut
        , typename MethodResult<barobo::Widget>::nullaryWithResultError
        , typename MethodResult<barobo::Widget>::nullaryWithResult
        , typename MethodResult<barobo::Widget>::unaryNoResult
        , typename MethodResult<barobo::Widget>::unaryWithResultOut
        , typename MethodResult<barobo::Widget>::unaryWithResultError
        , typename MethodResult<barobo::Widget>::unaryWithResult
        >;
};

/* COMPONENT IDs */

template <>
struct ComponentId<barobo::Widget> {
#define ITEM(x) x = hash(#x)
    enum : uint32_t {
        ITEM(attribute),
        ITEM(readonlyAttribute),
        ITEM(noSubscriptionsAttribute),
        ITEM(readonlyNoSubscriptionsAttribute),
        ITEM(nullaryNoResult),
        ITEM(nullaryWithResultOut),
        ITEM(nullaryWithResultError),
        ITEM(nullaryWithResult),
        ITEM(unaryNoResult),
        ITEM(unaryWithResultOut),
        ITEM(unaryWithResultError),
        ITEM(unaryWithResult),
        ITEM(broadcast)
    };
#undef ITEM
};

template <>
constexpr uint32_t componentId (Attribute<barobo::Widget>::attribute) {
    return ComponentId<barobo::Widget>::attribute;
}
template <>
constexpr uint32_t componentId (Attribute<barobo::Widget>::readonlyAttribute) {
    return ComponentId<barobo::Widget>::readonlyAttribute;
}
template <>
constexpr uint32_t componentId (Attribute<barobo::Widget>::noSubscriptionsAttribute) {
    return ComponentId<barobo::Widget>::noSubscriptionsAttribute;
}
template <>
constexpr uint32_t componentId (Attribute<barobo::Widget>::readonlyNoSubscriptionsAttribute) {
    return ComponentId<barobo::Widget>::readonlyNoSubscriptionsAttribute;
}

template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::nullaryNoResult) {
    return ComponentId<barobo::Widget>::nullaryNoResult;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::nullaryWithResultOut) {
    return ComponentId<barobo::Widget>::nullaryWithResultOut;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::nullaryWithResultError) {
    return ComponentId<barobo::Widget>::nullaryWithResultError;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::nullaryWithResult) {
    return ComponentId<barobo::Widget>::nullaryWithResult;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::unaryNoResult) {
    return ComponentId<barobo::Widget>::unaryNoResult;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::unaryWithResultOut) {
    return ComponentId<barobo::Widget>::unaryWithResultOut;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::unaryWithResultError) {
    return ComponentId<barobo::Widget>::unaryWithResultError;
}
template <>
constexpr uint32_t componentId (MethodIn<barobo::Widget>::unaryWithResult) {
    return ComponentId<barobo::Widget>::unaryWithResult;
}
template <>
constexpr uint32_t componentId (MethodResult<barobo::Widget>::unaryNoResult) {
    return ComponentId<barobo::Widget>::unaryNoResult;
}
template <>
constexpr uint32_t componentId (MethodResult<barobo::Widget>::unaryWithResultOut) {
    return ComponentId<barobo::Widget>::unaryWithResultOut;
}
template <>
constexpr uint32_t componentId (MethodResult<barobo::Widget>::unaryWithResultError) {
    return ComponentId<barobo::Widget>::unaryWithResultError;
}
template <>
constexpr uint32_t componentId (MethodResult<barobo::Widget>::unaryWithResult) {
    return ComponentId<barobo::Widget>::unaryWithResult;
}

template <>
class Subscriptions<barobo::Widget> {
public:
    using Id = ComponentId<barobo::Widget>;

    Status activate (uint32_t id) {
        switch (id) {
            case Id::attribute:
                attribute = true;
                return Status::OK;
            case Id::readonlyAttribute:
                readonlyAttribute = true;
                return Status::OK;
#if 0
            case Id::noSubscriptionsAttribute:
                noSubscriptionsAttribute = true;
                return Status::OK;
            case Id::readonlyNoSubscriptionsAttribute:
                readonlyNoSubscriptionsAttribute = true;
                return Status::OK;
#endif
            case Id::broadcast:
                broadcast = true;
                return Status::OK;
            default:
                if (isAttribute<barobo::Widget>(id)) {
                    return Status::NO_SUBSCRIPTIONS;
                }
                return isComponent<barobo::Widget>(id) ?
                           Status::ILLEGAL_OPERATION :
                           Status::NO_SUCH_COMPONENT;
        }
    }

    Status deactivate (uint32_t id) {
        switch (id) {
            case Id::attribute:
                attribute = false;
                return Status::OK;
            case Id::readonlyAttribute:
                readonlyAttribute = false;
                return Status::OK;
#if 0
            case Id::noSubscriptionsAttribute:
                noSubscriptionsAttribute = false;
                return Status::OK;
            case Id::readonlyNoSubscriptionsAttribute:
                readonlyNoSubscriptionsAttribute = false;
                return Status::OK;
#endif
            case Id::broadcast:
                broadcast = true;
                return Status::OK;
            default:
                if (isAttribute<barobo::Widget>(id)) {
                    return Status::NO_SUBSCRIPTIONS;
                }
                return isComponent<barobo::Widget>(id) ?
                           Status::ILLEGAL_OPERATION :
                           Status::NO_SUCH_COMPONENT;
        }
    }

    bool isActive (uint32_t id) {
        switch (id) {
            case Id::attribute:
                return attribute;
            case Id::readonlyAttribute:
                return readonlyAttribute;
#if 0
            case Id::noSubscriptionsAttribute:
                return noSubscriptionsAttribute;
            case Id::readonlyNoSubscriptionsAttribute:
                return readonlyNoSubscriptionsAttribute;
#endif
            case Id::broadcast:
                return broadcast;
            default:
                return false;
        }
    }

    void reset () {
        attribute = false;
        readonlyAttribute = false;
#if 0
        noSubscriptionsAttribute = false;
        readonlyNoSubscriptionsAttribute = false;
#endif
        broadcast = false;
    }

private:
    bool attribute = false;
    bool readonlyAttribute = false;
#if 0
    bool noSubscriptionsAttribute = false;
    bool readonlyNoSubscriptionsAttribute = false;
#endif
    bool broadcast = false;
};

template <>
struct GetInvoker<barobo::Widget> {
    template <class T>
    static Status invoke (T& service,
            ComponentResultUnion<barobo::Widget>& argument,
            uint32_t componentId,
            barobo_rpc_Reply_Result_payload_t& payload) {
        /* TODO: static_assert that T implements barobo::Widget */
        using Id = ComponentId<barobo::Widget>;
        switch (componentId) {
            // list of attributes
            case Id::attribute: {
                auto val = service.get(argument.attribute);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::readonlyAttribute: {
                auto val = service.get(argument.readonlyAttribute);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::noSubscriptionsAttribute: {
                auto val = service.get(argument.noSubscriptionsAttribute);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::readonlyNoSubscriptionsAttribute: {
                auto val = service.get(argument.readonlyNoSubscriptionsAttribute);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            default:
                return isComponent<barobo::Widget>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct SetInvoker<barobo::Widget> {
    template <class T>
    static Status invoke (T& service,
            ComponentInUnion<barobo::Widget>& argument,
            uint32_t componentId) {
        /* TODO: static_assert that T implements barobo::Widget */
        using Id = ComponentId<barobo::Widget>;
        switch (componentId) {
            // list of non-read-only attributes
            case Id::attribute:
                service.set(argument.attribute);
                return Status::OK;
#if 0
            case Id::readonlyAttribute:
                service.set(argument.readonlyAttribute);
                return Status::OK;
#endif
            case Id::noSubscriptionsAttribute:
                service.set(argument.noSubscriptionsAttribute);
                return Status::OK;
#if 0
            case Id::readonlyNoSubscriptionsAttribute:
                service.set(argument.readonlyNoSubscriptionsAttribute);
                return Status::OK;
#endif
            default:
                if (isAttribute<barobo::Widget>(componentId)) {
                    return Status::READONLY;
                }
                return isComponent<barobo::Widget>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct FireInvoker<barobo::Widget> {
    template <class T>
    static Status invoke (T& service,
            ComponentInUnion<barobo::Widget>& argument,
            uint32_t componentId,
            barobo_rpc_Reply_Result_payload_t& payload) {
        /* TODO: static_assert that T implements barobo::Widget */
        using Id = ComponentId<barobo::Widget>;
        switch (componentId) {
            // list of methods
            case Id::nullaryNoResult: {
                auto val = service.fire(argument.nullaryNoResult);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::nullaryWithResultOut: {
                auto val = service.fire(argument.nullaryWithResultOut);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::nullaryWithResultError: {
                auto val = service.fire(argument.nullaryWithResultError);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::nullaryWithResult: {
                auto val = service.fire(argument.nullaryWithResult);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::unaryNoResult: {
                auto val = service.fire(argument.unaryNoResult);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::unaryWithResultOut: {
                auto val = service.fire(argument.unaryWithResultOut);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::unaryWithResultError: {
                auto val = service.fire(argument.unaryWithResultError);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            case Id::unaryWithResult: {
                auto val = service.fire(argument.unaryWithResult);
                payload.size = sizeof(payload.bytes);
                return encodeProtobuf(&val, pbFields(val), payload.bytes, payload.size, payload.size);
            }
            default:
                return isComponent<barobo::Widget>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct BroadcastInvoker<barobo::Widget> {
    template <class T>
    static Status invoke (T& proxy,
            ComponentResultUnion<barobo::Widget>& argument,
            uint32_t componentId) {
        /* TODO: static_assert that T implements barobo::Widget */
        using Id = ComponentId<barobo::Widget>;
        switch (componentId) {
            // list of subscribable attributes
            case Id::attribute:
                proxy.broadcast(argument.attribute);
                return Status::OK;
            case Id::readonlyAttribute:
                proxy.broadcast(argument.readonlyAttribute);
                return Status::OK;
#if 0
            case Id::noSubscriptionsAttribute:
                proxy.broadcast(argument.noSubscriptionsAttribute);
                return Status::OK;
            case Id::readonlyNoSubscriptionsAttribute:
                proxy.broadcast(argument.readonlyNoSubscriptionsAttribute);
                return Status::OK;
#endif
            // list of broadcasts
            case Id::broadcast:
                proxy.broadcast(argument.broadcast);
                return Status::OK;
            default:
                if (isAttribute<barobo::Widget>(componentId)) {
                    return Status::NO_SUBSCRIPTIONS;
                }
                return isComponent<barobo::Widget>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

template <>
struct FulfillInvoker<barobo::Widget> {
    template <class T>
    static Status invoke (T& proxy,
            ComponentResultUnion<barobo::Widget>& argument,
            uint32_t componentId,
            uint32_t requestId) {
        /* TODO: static_assert that T implements barobo::Widget */
        using Id = ComponentId<barobo::Widget>;
        switch (componentId) {
            // list of attributes
            case Id::attribute:
                return proxy.fulfill(requestId, argument.attribute);
            case Id::readonlyAttribute:
                return proxy.fulfill(requestId, argument.readonlyAttribute);
            case Id::noSubscriptionsAttribute:
                return proxy.fulfill(requestId, argument.noSubscriptionsAttribute);
            case Id::readonlyNoSubscriptionsAttribute:
                return proxy.fulfill(requestId, argument.readonlyNoSubscriptionsAttribute);
            // list of non-fire-and-forget methods
            case Id::nullaryNoResult:
                return proxy.fulfill(requestId, argument.nullaryNoResult);
            case Id::nullaryWithResultOut:
                return proxy.fulfill(requestId, argument.nullaryWithResultOut);
            case Id::nullaryWithResultError:
                return proxy.fulfill(requestId, argument.nullaryWithResultError);
            case Id::nullaryWithResult:
                return proxy.fulfill(requestId, argument.nullaryWithResult);
            case Id::unaryNoResult:
                return proxy.fulfill(requestId, argument.unaryNoResult);
            case Id::unaryWithResultOut:
                return proxy.fulfill(requestId, argument.unaryWithResultOut);
            case Id::unaryWithResultError:
                return proxy.fulfill(requestId, argument.unaryWithResultError);
            case Id::unaryWithResult:
                return proxy.fulfill(requestId, argument.unaryWithResult);
            default:
                return isBroadcast<barobo::Widget>(componentId) ?
                    Status::ILLEGAL_OPERATION :
                    Status::NO_SUCH_COMPONENT;
        }
    }
};

} // namespace rpc

#endif
