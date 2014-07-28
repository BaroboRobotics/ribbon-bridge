#ifndef RPC_DEF_HPP
#define RPC_DEF_HPP

#include "rpc/checkversion.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/hash.hpp"
#include "rpc/message.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/reverse.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>

//////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION MACROS


//////////////////////////////////////////////////////////////////////////////
// pbFields

#define rpcdef_pbFields(r, prefix, component) \
    template <> \
    const pb_field_t* pbFields (BOOST_PP_CAT(prefix, component)) { \
        return BOOST_PP_CAT(prefix, BOOST_PP_CAT(component, _fields)); \
    }

#define add_suffix(s, suffix, elem) BOOST_PP_CAT(elem, suffix)
#define add_prefix(s, prefix, elem) BOOST_PP_CAT(prefix, elem)

#define RPCDEF_pbFields_attributes(prefix, attributes) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_pbFields, prefix, attributes)

#define RPCDEF_pbFields_methods(prefix, methods) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_pbFields, prefix, \
            BOOST_PP_SEQ_TRANSFORM(add_suffix, _In, methods)) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_pbFields, prefix, \
            BOOST_PP_SEQ_TRANSFORM(add_suffix, _Result, methods))

#define RPCDEF_pbFields_broadcasts RPCDEF_pbFields_attributes

//////////////////////////////////////////////////////////////////////////////
// isAttribute, isMethod, isBroadcast

#define rpcdef_isAttributeMethodOrBroadcast(r, data, component) \
    case Id::component:

#define RPCDEF_isAttributeMethodOrBroadcast(functionName, interface, components) \
    template <> \
    bool functionName<interface> (uint32_t id) { \
        using Id = ComponentId<interface>; \
        switch (id) { \
            BOOST_PP_SEQ_FOR_EACH(rpcdef_isAttributeMethodOrBroadcast, ~, components) \
            return true; \
        default: \
            return false; \
        } \
    }

#define RPCDEF_isAttribute(interface, attributes) \
    RPCDEF_isAttributeMethodOrBroadcast(isAttribute, interface, attributes)
#define RPCDEF_isMethod(interface, methods) \
    RPCDEF_isAttributeMethodOrBroadcast(isMethod, interface, methods)
#define RPCDEF_isBroadcast(interface, broadcasts) \
    RPCDEF_isAttributeMethodOrBroadcast(isBroadcast, interface, broadcasts)

//////////////////////////////////////////////////////////////////////////////
// decode functions

#define rpcdef_decodeCase(s, data, component) \
    case Id::component: \
        return decode(args.component, payload.bytes, payload.size);

#define RPCDEF_decodeSetPayload(interface, settableAttributes) \
    template <> \
    Status decodeSetPayload (ComponentInUnion<interface>& args, \
            uint32_t componentId, \
            barobo_rpc_Request_Set_payload_t& payload) { \
        using Id = ComponentId<interface>; \
        switch (componentId) { \
            BOOST_PP_SEQ_FOR_EACH(rpcdef_decodeCase, ~, settableAttributes) \
        default: \
             if (isAttribute<interface>(componentId)) { \
                 return Status::READONLY; \
             } \
            return isComponent<interface>(componentId) ? \
                Status::ILLEGAL_OPERATION : \
                Status::NO_SUCH_COMPONENT; \
        } \
    }

#define RPCDEF_decodeFirePayload(interface, methods) \
    template <> \
    Status decodeFirePayload (ComponentInUnion<interface>& args, \
            uint32_t componentId, \
            barobo_rpc_Request_Fire_payload_t& payload) { \
        using Id = ComponentId<interface>; \
        switch (componentId) { \
            BOOST_PP_SEQ_FOR_EACH(rpcdef_decodeCase, ~, methods) \
        default: \
            return isComponent<interface>(componentId) ? \
                Status::ILLEGAL_OPERATION : \
                Status::NO_SUCH_COMPONENT; \
        } \
    }

#define RPCDEF_decodeBroadcastPayload(interface, broadcasts) \
    template <> \
    Status decodeBroadcastPayload (ComponentResultUnion<interface>& args, \
            uint32_t componentId, \
            barobo_rpc_Reply_Broadcast_payload_t& payload) { \
        using Id = ComponentId<interface>; \
        switch (componentId) { \
            BOOST_PP_SEQ_FOR_EACH(rpcdef_decodeCase, ~, broadcasts) \
            default: \
                if (isAttribute<interface>(componentId)) { \
                    return Status::NO_SUBSCRIPTIONS; \
                } \
                return isComponent<interface>(componentId) ? \
                    Status::ILLEGAL_OPERATION : \
                    Status::NO_SUCH_COMPONENT; \
        } \
    }

#define RPCDEF_decodeResultPayload(interface, components) \
    template <> \
    Status decodeResultPayload (ComponentResultUnion<interface>& args, \
            uint32_t componentId, \
            barobo_rpc_Reply_Result_payload_t& payload) { \
        using Id = ComponentId<interface>; \
        switch (componentId) { \
            BOOST_PP_SEQ_FOR_EACH(rpcdef_decodeCase, ~, components) \
            default: \
                return isComponent<interface>(componentId) ? \
                    Status::ILLEGAL_OPERATION : \
                    Status::NO_SUCH_COMPONENT; \
        } \
    }

//////////////////////////////////////////////////////////////////////////////
// getSubscriptionRecord
//
#define rpcdef_case_subscription_record(s, subscriptions, subscribable) \
        case Id::subscribable: return subscriptions.subscribable;

#define RPCDEF_getSubscriptionRecord(interface, subscribables) \
    template <> \
    bool& getSubscriptionRecord (Subscriptions<interface>& subs, uint32_t id, bool& dummy, Status& status) { \
        using Id = ComponentId<interface>; \
        status = Status::OK; \
        switch (id) { \
            BOOST_PP_SEQ_FOR_EACH(rpcdef_case_subscription_record, subs, subscribables) \
            default: \
                if (isAttribute<interface>(id)) { \
                    status = Status::NO_SUBSCRIPTIONS; \
                } \
                status = isComponent<interface>(id) ? \
                           Status::ILLEGAL_OPERATION : \
                           Status::NO_SUCH_COMPONENT; \
                return dummy; \
        } \
    }


//////////////////////////////////////////////////////////////////////////////
// HEADER MACROS


//////////////////////////////////////////////////////////////////////////////
// Version

#define intercalate_scope(s, z, x) z::x

#define rpcdef_cat_scope_seq(atoms) BOOST_PP_SEQ_FOLD_LEFT(intercalate_scope, \
        BOOST_PP_SEQ_HEAD(atoms), \
        BOOST_PP_SEQ_TAIL(atoms))

#define rpcdef_cat_scope(atoms) rpcdef_cat_scope_seq(BOOST_PP_TUPLE_TO_SEQ(atoms))

#define RPCDEF_Version(interface, version) \
    template <> \
    struct Version<interface> { \
        constexpr static const uint32_t major = BOOST_PP_TUPLE_ELEM(0, version); \
        constexpr static const uint32_t minor = BOOST_PP_TUPLE_ELEM(1, version); \
        constexpr static const uint32_t patch = BOOST_PP_TUPLE_ELEM(2, version); \
    };

#define RPCDEF_FWD_DECL_isComponentFunctions(interface) \
    template <> \
    bool isAttribute<interface> (uint32_t id); \
    template <> \
    bool isMethod<interface> (uint32_t id); \
    template <> \
    bool isBroadcast<interface> (uint32_t id);

//////////////////////////////////////////////////////////////////////////////
// Forward declare interface struct

#define enclose_with_namespace(s, z, x) namespace x { z }

#define rpcdef_fwd_decl_struct_seq_rev(rnames) BOOST_PP_SEQ_FOLD_LEFT(enclose_with_namespace, \
        struct BOOST_PP_SEQ_HEAD(rnames);, BOOST_PP_SEQ_TAIL(rnames))

#define rpcdef_fwd_decl_struct_seq(names) rpcdef_fwd_decl_struct_seq_rev(BOOST_PP_SEQ_REVERSE(names))

// XXX This macro must be invoked at global namespace scope!
#define RPCDEF_FWD_DECL_INTERFACE(names) rpcdef_fwd_decl_struct_seq(BOOST_PP_TUPLE_TO_SEQ(names))

//////////////////////////////////////////////////////////////////////////////
// Build an underscored prefix, i.e. (barobo, Widget) becomes barobo_Widget_
#define rpcdef_paste(s, z, x) BOOST_PP_CAT(z, x)
#define rpcdef_join_seq(atoms) BOOST_PP_SEQ_FOLD_LEFT(rpcdef_paste, \
        BOOST_PP_SEQ_HEAD(atoms), \
        BOOST_PP_SEQ_TAIL(atoms))

#define rcpdef_append(s, suffix, x) BOOST_PP_CAT(x, suffix)
#define rpcdef_underscored_token_seq(atoms) rpcdef_join_seq(BOOST_PP_SEQ_TRANSFORM(rcpdef_append, _, atoms))

#define rpcdef_underscored_token(atoms) rpcdef_underscored_token_seq(BOOST_PP_TUPLE_TO_SEQ(atoms))

//////////////////////////////////////////////////////////////////////////////
// Attribute metafunctions

#define rpcdef_using(s, prefix, x) using x = BOOST_PP_CAT(prefix, x);

#define RPCDEF_Attribute(interfaceNames, attributes) \
    template <> \
    struct Attribute<rpcdef_cat_scope(interfaceNames)> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_using, \
                rpcdef_underscored_token(interfaceNames), \
                attributes) \
    };

#define rpcdef_define_true_metafunc(s, metafuncname, componentStruct) \
    template <> \
    struct metafuncname<componentStruct> { \
        constexpr static const bool value = true; \
    };

#define rpcdef_make_attribute_struct(s, interface, attribute) \
    Attribute<interface>::attribute
#define rpcdef_make_method_input_struct(s, interface, method) \
    MethodIn<interface>::method
#define rpcdef_make_method_output_struct(s, interface, method) \
    MethodResult<interface>::method
#define rpcdef_make_broadcast_struct(s, interface, broadcast) \
    Broadcast<interface>::broadcast

#define rpcdef_make_typename_attribute_struct(s, interface, attribute) \
    typename Attribute<interface>::attribute
#define rpcdef_make_typename_method_input_struct(s, interface, method) \
    typename MethodIn<interface>::method
#define rpcdef_make_typename_method_output_struct(s, interface, method) \
    typename MethodResult<interface>::method
#define rpcdef_make_typename_broadcast_struct(s, interface, broadcast) \
    typename Broadcast<interface>::broadcast


#define RPCDEF_IsAttribute(interface, attributes) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_true_metafunc, IsAttribute, \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_attribute_struct, \
                interface, attributes))

#define RPCDEF_IsSettableAttribute(interface, attributes) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_true_metafunc, IsSettableAttribute, \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_attribute_struct, \
                interface, attributes))

#define RPCDEF_IsSubscribableAttribute(interface, attributes) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_true_metafunc, IsSubscribableAttribute, \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_attribute_struct, \
                interface, attributes))

//////////////////////////////////////////////////////////////////////////////
// Method metafunctions

#define rpcdef_using_In(s, prefix, x) using x = BOOST_PP_CAT(prefix, BOOST_PP_CAT(x, _In));
#define rpcdef_using_Result(s, prefix, x) using x = BOOST_PP_CAT(prefix, BOOST_PP_CAT(x, _Result));

#define RPCDEF_MethodIn(interfaceNames, methods) \
    template <> \
    struct MethodIn<rpcdef_cat_scope(interfaceNames)> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_using_In, \
                rpcdef_underscored_token(interfaceNames), \
                methods) \
    };

#define RPCDEF_MethodResult(interfaceNames, methods) \
    template <> \
    struct MethodResult<rpcdef_cat_scope(interfaceNames)> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_using_Result, \
                rpcdef_underscored_token(interfaceNames), \
                methods) \
    };

#define rpcdef_define_ResultOf(s, interface, method) \
    template <> \
    struct ResultOf<MethodIn<interface>::method> { \
        using type = MethodResult<interface>::method; \
    };

#define RPCDEF_ResultOf(interface, methods) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_ResultOf, interface, \
            methods)


// Note that the two transformations in this macro are concatenated, they are
// not separate arguments to SEQ_FOR_EACH.
#define RPCDEF_IsMethod(interface, methods) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_true_metafunc, IsMethod, \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_method_input_struct, \
                interface, methods) \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_method_output_struct, \
                interface, methods))

//////////////////////////////////////////////////////////////////////////////
// Broadcast metafunctions

#define RPCDEF_Broadcast(interfaceNames, broadcasts) \
    template <> \
    struct Broadcast<rpcdef_cat_scope(interfaceNames)> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_using, \
                rpcdef_underscored_token(interfaceNames), \
                broadcasts) \
    };

#define RPCDEF_IsBroadcast(interface, broadcasts) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_true_metafunc, IsBroadcast, \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_broadcast_struct, \
                interface, broadcasts))

//////////////////////////////////////////////////////////////////////////////
// Component Unions

#define rpcdef_decl_attribute_object(s, interface, attribute)  typename Attribute<interface>::attribute attribute;
#define rpcdef_decl_method_input_object(s, interface, method) typename MethodIn<interface>::method method;

#define RPCDEF_ComponentInUnion(interface, settableAttributes, methods) \
    template <> \
    union ComponentInUnion<interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_attribute_object, interface, settableAttributes) \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_method_input_object, interface, methods) \
    };

#define rpcdef_decl_method_output_object(s, interface, method) typename MethodResult<interface>::method method;
#define rpcdef_decl_broadcast_object(s, interface, broadcast)  typename Broadcast<interface>::broadcast broadcast;

#define RPCDEF_ComponentResultUnion(interface, attributes, methods, broadcasts) \
    template <> \
    union ComponentResultUnion<interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_attribute_object, interface, attributes) \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_method_output_object, interface, methods) \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_broadcast_object, interface, broadcasts) \
    };

//////////////////////////////////////////////////////////////////////////////
// PromiseVariadic, a metafunction to help create variants of promises for
// requests that have results. Use like so:
//   template <class... T>
//   using MakePromiseVariant = Variant<Promise<T>...>;
//   using PromiseVariant = typename PromiseVariadic<Interface, MakePromiseVariant>::type;
// and PromiseVariant will be a variant of promises for all attributes, method
// results, and the void type.

#define RPCDEF_PromiseVariadic(interface, attributes, methods) \
    template <template <class...> class F> \
    struct PromiseVariadic<interface, F> { \
        using type = F \
            < \
            BOOST_PP_SEQ_ENUM( \
                    (void) \
                    BOOST_PP_SEQ_TRANSFORM( \
                        rpcdef_make_typename_attribute_struct, \
                        interface, attributes) \
                    BOOST_PP_SEQ_TRANSFORM( \
                        rpcdef_make_typename_method_output_struct, \
                        interface, methods)) \
            >; \
    };
        
//////////////////////////////////////////////////////////////////////////////
// Component IDs

#define rpcdef_make_enum_hash_entry(s, z, component) \
        component = hash(BOOST_PP_STRINGIZE(component))

#define RPCDEF_ComponentId(interface, components) \
    template <> \
    struct ComponentId<interface> { \
        enum : uint32_t { \
            BOOST_PP_SEQ_ENUM( \
                    BOOST_PP_SEQ_TRANSFORM( \
                        rpcdef_make_enum_hash_entry, \
                        ~, components)) \
        }; \
    };

#define rpcdef_define_componentId(type, interface, component) \
    template <> \
    constexpr uint32_t componentId (type<interface>::component) { \
        return ComponentId<interface>::component; \
    }
    
#define rpcdef_attribute_componentId(s, interface, attribute) \
    rpcdef_define_componentId(Attribute, interface, attribute)

#define rpcdef_method_componentId(s, interface, method) \
    rpcdef_define_componentId(MethodIn, interface, method)

#define rpcdef_broadcast_componentId(s, interface, broadcast) \
    rpcdef_define_componentId(Broadcast, interface, broadcast)

#define RPCDEF_componentId(interface, attributes, methods, broadcasts) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_attribute_componentId, interface, attributes) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_method_componentId, interface, methods) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_broadcast_componentId, interface, broadcasts)

//////////////////////////////////////////////////////////////////////////////
// Subscriptions

#define rpcdef_define_subscription(s, z, subscribable) \
    bool subscribable = false;

#define RPCDEF_Subscriptions(interface, subscribables) \
    template <> \
    struct Subscriptions<interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_define_subscription, ~, subscribables) \
    };

//////////////////////////////////////////////////////////////////////////////
// Invokers

#define rpcdef_case_invoke_get(s, z, attribute) \
    case Id::attribute: \
        return encode(service.get(argument.attribute), payload.bytes, payload.size, payload.size);

// TODO: static_assert that T implements the interface!
#define RPCDEF_GetInvoker(interface, attributes) \
    template <> \
    struct GetInvoker<interface> { \
        template <class T> \
        static Status invoke (T& service, \
                ComponentResultUnion<interface>& argument, \
                uint32_t componentId, \
                barobo_rpc_Reply_Result_payload_t& payload) { \
            using Id = ComponentId<interface>; \
            payload.size = sizeof(payload.bytes); \
            switch (componentId) { \
                BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_get, ~, attributes) \
                default: \
                    return isComponent<interface>(componentId) ? \
                        Status::ILLEGAL_OPERATION : \
                        Status::NO_SUCH_COMPONENT; \
            } \
        } \
    };

#define rpcdef_case_invoke_set(s, z, attribute) \
    case Id::attribute: \
        service.set(argument.attribute); \
        return Status::OK;

// TODO: static_assert that T implements barobo::Widget
#define RPCDEF_SetInvoker(interface, settableAttributes) \
    template <> \
    struct SetInvoker<interface> { \
        template <class T> \
        static Status invoke (T& service, \
                ComponentInUnion<interface>& argument, \
                uint32_t componentId) { \
            using Id = ComponentId<interface>; \
            switch (componentId) { \
                BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_set, ~, settableAttributes) \
                default: \
                    if (isAttribute<interface>(componentId)) { \
                        return Status::READONLY; \
                    } \
                    return isComponent<interface>(componentId) ? \
                        Status::ILLEGAL_OPERATION : \
                        Status::NO_SUCH_COMPONENT; \
            } \
        } \
    };

#define rpcdef_case_invoke_fire(s, z, method) \
    case Id::method: \
        return encode(service.fire(argument.method), payload.bytes, payload.size, payload.size);

// TODO: static_assert that T implements the interface!
#define RPCDEF_FireInvoker(interface, methods) \
    template <> \
    struct FireInvoker<interface> { \
        template <class T> \
        static Status invoke (T& service, \
                ComponentInUnion<interface>& argument, \
                uint32_t componentId, \
                barobo_rpc_Reply_Result_payload_t& payload) { \
            using Id = ComponentId<interface>; \
            payload.size = sizeof(payload.bytes); \
            switch (componentId) { \
                BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_fire, ~, methods) \
                default: \
                    return isComponent<interface>(componentId) ? \
                        Status::ILLEGAL_OPERATION : \
                        Status::NO_SUCH_COMPONENT; \
            } \
        } \
    };

#define rpcdef_case_invoke_broadcast(s, z, brdcst) \
    case Id::brdcst: \
        proxy.broadcast(argument.brdcst); \
        return Status::OK;

// TODO: static_assert that T implements the interface!
#define RPCDEF_BroadcastInvoker(interface, broadcasts) \
    template <> \
    struct BroadcastInvoker<interface> { \
        template <class T> \
        static Status invoke (T& proxy, \
                ComponentResultUnion<interface>& argument, \
                uint32_t componentId) { \
            using Id = ComponentId<interface>; \
            switch (componentId) { \
                BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_broadcast, ~, broadcasts) \
                default: \
                    if (isAttribute<interface>(componentId)) { \
                        return Status::NO_SUBSCRIPTIONS; \
                    } \
                    return isComponent<interface>(componentId) ? \
                        Status::ILLEGAL_OPERATION : \
                        Status::NO_SUCH_COMPONENT; \
            } \
        } \
    };

#define rpcdef_case_invoke_fulfill(s, z, promisable) \
    case Id::promisable: \
        return proxy.fulfill(requestId, argument.promisable);

// TODO: static_assert that T implements the interface!
#define RPCDEF_FulfillInvoker(interface, promisables) \
    template <> \
    struct FulfillInvoker<interface> { \
        template <class T> \
        static Status invoke (T& proxy, \
                ComponentResultUnion<interface>& argument, \
                uint32_t componentId, \
                uint32_t requestId) { \
            using Id = ComponentId<interface>; \
            switch (componentId) { \
                BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_fulfill, ~, promisables) \
                default: \
                    return isBroadcast<interface>(componentId) ? \
                        Status::ILLEGAL_OPERATION : \
                        Status::NO_SUCH_COMPONENT; \
            } \
        } \
    };

//////////////////////////////////////////////////////////////////////////////
// Compile-time assertions that a proxy implements an interface

#define rpcdef_proxy_attribute_assertion(s, interface, attribute) \
    static_assert(HasMemberFunctionOverloadonBroadcast \
            < T \
            , void(rpc::Attribute<interface>::attribute)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " proxy must implement onBroadcast(" \
            BOOST_PP_STRINGIZE(attribute) ")");

#define rpcdef_proxy_broadcast_assertion(s, interface, broadcast) \
    static_assert(HasMemberFunctionOverloadonBroadcast \
            < T \
            , void(rpc::Broadcast<interface>::broadcast)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " proxy must implement onBroadcast(" \
            BOOST_PP_STRINGIZE(broadcast) ")");

#define RPCDEF_AssertProxyImplementsInterface(interface, subscribableAttributes, broadcasts) \
    template <class T> \
    struct AssertProxyImplementsInterface<T, interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_proxy_attribute_assertion, interface, subscribableAttributes) \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_proxy_broadcast_assertion, interface, broadcasts) \
    };

//////////////////////////////////////////////////////////////////////////////
// Compile-time assertions that a service implements an interface

#define rpcdef_service_get_assertion(s, interface, attribute) \
    static_assert(HasMemberFunctionOverloadonGet \
            < T \
            , rpc::Attribute<interface>::attribute(rpc::Attribute<interface>::attribute)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " service does not implement onGet(" \
            BOOST_PP_STRINGIZE(attribute) ")");

#define rpcdef_service_set_assertion(s, interface, attribute) \
    static_assert(HasMemberFunctionOverloadonSet \
            < T \
            , void(rpc::Attribute<interface>::attribute)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " service does not implement onSet(" \
            BOOST_PP_STRINGIZE(attribute) ")");

#define rpcdef_service_fire_assertion(s, interface, method) \
    static_assert(HasMemberFunctionOverloadonFire \
            < T \
            , rpc::MethodResult<interface>::method(rpc::MethodIn<interface>::method)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " service does not implement onFire(" \
            BOOST_PP_STRINGIZE(method) ")");

#define RPCDEF_AssertServiceImplementsInterface(interface, allAttributes, settableAttributes, methods) \
    template <class T> \
    struct AssertServiceImplementsInterface<T, interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_service_get_assertion, interface, allAttributes) \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_service_set_assertion, interface, settableAttributes) \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_service_fire_assertion, interface, methods) \
    };

//////////////////////////////////////////////////////////////////////////////
// Complete header and cpp file defines

#define RPCDEF_CPP(interfaceNames, allAttributes, settableAttributes, subscribableAttributes, methods, broadcasts) \
    namespace rpc { \
    RPCDEF_pbFields_attributes(rpcdef_underscored_token(interfaceNames), allAttributes) \
    RPCDEF_pbFields_methods(rpcdef_underscored_token(interfaceNames), methods) \
    RPCDEF_pbFields_broadcasts(rpcdef_underscored_token(interfaceNames), broadcasts) \
    RPCDEF_isAttribute(rpcdef_cat_scope(interfaceNames), allAttributes) \
    RPCDEF_isMethod(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_isBroadcast(rpcdef_cat_scope(interfaceNames), broadcasts) \
    RPCDEF_decodeSetPayload(rpcdef_cat_scope(interfaceNames), settableAttributes) \
    RPCDEF_decodeFirePayload(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_decodeBroadcastPayload(rpcdef_cat_scope(interfaceNames), subscribableAttributes broadcasts) \
    RPCDEF_decodeResultPayload(rpcdef_cat_scope(interfaceNames), allAttributes methods) \
    RPCDEF_getSubscriptionRecord(rpcdef_cat_scope(interfaceNames), subscribableAttributes broadcasts) \
    }

#define RPCDEF_HPP(interfaceNames, version, allAttributes, settableAttributes, subscribableAttributes, methods, broadcasts) \
    RPCDEF_FWD_DECL_INTERFACE(interfaceNames) \
    namespace rpc { \
    RPCDEF_Version(rpcdef_cat_scope(interfaceNames), version) \
    RPCDEF_FWD_DECL_isComponentFunctions(rpcdef_cat_scope(interfaceNames)) \
    RPCDEF_Attribute(interfaceNames, allAttributes) \
    RPCDEF_IsAttribute(rpcdef_cat_scope(interfaceNames), allAttributes) \
    RPCDEF_IsSettableAttribute(rpcdef_cat_scope(interfaceNames), settableAttributes) \
    RPCDEF_IsSubscribableAttribute(rpcdef_cat_scope(interfaceNames), subscribableAttributes) \
    RPCDEF_MethodIn(interfaceNames, methods) \
    RPCDEF_MethodResult(interfaceNames, methods) \
    RPCDEF_ResultOf(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_IsMethod(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_Broadcast(interfaceNames, broadcasts) \
    RPCDEF_IsBroadcast(rpcdef_cat_scope(interfaceNames), broadcasts) \
    RPCDEF_ComponentInUnion(rpcdef_cat_scope(interfaceNames), settableAttributes, methods) \
    RPCDEF_ComponentResultUnion(rpcdef_cat_scope(interfaceNames), allAttributes, methods, broadcasts) \
    RPCDEF_PromiseVariadic(rpcdef_cat_scope(interfaceNames), allAttributes, methods) \
    RPCDEF_ComponentId(rpcdef_cat_scope(interfaceNames), allAttributes methods broadcasts) \
    RPCDEF_componentId(rpcdef_cat_scope(interfaceNames), allAttributes, methods, broadcasts) \
    RPCDEF_Subscriptions(rpcdef_cat_scope(interfaceNames), subscribableAttributes broadcasts) \
    RPCDEF_GetInvoker(rpcdef_cat_scope(interfaceNames), allAttributes) \
    RPCDEF_SetInvoker(rpcdef_cat_scope(interfaceNames), settableAttributes) \
    RPCDEF_FireInvoker(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_BroadcastInvoker(rpcdef_cat_scope(interfaceNames), subscribableAttributes broadcasts) \
    RPCDEF_FulfillInvoker(rpcdef_cat_scope(interfaceNames), allAttributes methods) \
    RPCDEF_AssertServiceImplementsInterface(rpcdef_cat_scope(interfaceNames), allAttributes, settableAttributes, methods) \
    RPCDEF_AssertProxyImplementsInterface(rpcdef_cat_scope(interfaceNames), subscribableAttributes, broadcasts) \
    }

#endif
