#ifndef RPC_DEF_HPP
#define RPC_DEF_HPP

#include <rpc/version.hpp>
#include <rpc/componenttraits.hpp>
#include <rpc/hash.hpp>
#include <rpc/message.hpp>

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
    const pb_field_t* pbFieldPtr<BOOST_PP_CAT(prefix, component)> () { \
        return BOOST_PP_CAT(prefix, BOOST_PP_CAT(component, _fields)); \
    }

#define add_suffix(s, suffix, elem) BOOST_PP_CAT(elem, suffix)
#define add_prefix(s, prefix, elem) BOOST_PP_CAT(prefix, elem)

#define RPCDEF_pbFields_methods(prefix, methods) \
    namespace _ { \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_pbFields, prefix, \
            BOOST_PP_SEQ_TRANSFORM(add_suffix, _In, methods)) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_pbFields, prefix, \
            BOOST_PP_SEQ_TRANSFORM(add_suffix, _Result, methods)) \
    }

#define RPCDEF_pbFields_broadcasts(prefix, broadcasts) \
    namespace _ { \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_pbFields, prefix, broadcasts) \
    }

//////////////////////////////////////////////////////////////////////////////
// HEADER MACROS


//////////////////////////////////////////////////////////////////////////////
// Version

#define RPCDEF_Version(interface, version) \
    template <> \
    struct Version<interface> { \
        static const uint32_t major = BOOST_PP_TUPLE_ELEM(0, version); \
        static const uint32_t minor = BOOST_PP_TUPLE_ELEM(1, version); \
        static const uint32_t patch = BOOST_PP_TUPLE_ELEM(2, version); \
        static VersionTriplet triplet () { return { major, minor, patch }; } \
    };

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


#define rpcdef_using(s, prefix, x) using x = BOOST_PP_CAT(prefix, x);

#define rpcdef_define_true_metafunc(s, metafuncname, componentStruct) \
    template <> \
    struct metafuncname<componentStruct> { \
        static const bool value = true; \
    };

//////////////////////////////////////////////////////////////////////////////
// Transform (a)(b)(c) to a::b::c.

#define intercalate_scope(s, z, x) z::x

#define rpcdef_cat_scope_seq(atoms) BOOST_PP_SEQ_FOLD_LEFT(intercalate_scope, \
        BOOST_PP_SEQ_HEAD(atoms), \
        BOOST_PP_SEQ_TAIL(atoms))

#define rpcdef_cat_scope(atoms) rpcdef_cat_scope_seq(BOOST_PP_TUPLE_TO_SEQ(atoms))

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


#define rpcdef_make_method_input_struct(s, interface, method) \
    MethodIn<interface>::method
#define rpcdef_make_method_output_struct(s, interface, method) \
    MethodResult<interface>::method

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

#define rpcdef_make_broadcast_struct(s, interface, broadcast) \
    Broadcast<interface>::broadcast

#define RPCDEF_IsBroadcast(interface, broadcasts) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_define_true_metafunc, IsBroadcast, \
            BOOST_PP_SEQ_TRANSFORM(rpcdef_make_broadcast_struct, \
                interface, broadcasts))

//////////////////////////////////////////////////////////////////////////////
// Component Unions

#define rpcdef_case_invoke_fire(s, interface, method) \
    case ::rpc::componentId(MethodIn<interface>::method{}): \
        decode(this->method, in.bytes, in.size, status); \
        if (!hasError(status)) { \
            encode(server.onFire(this->method), out.bytes, sizeof(out.bytes), out.size, status); \
        } \
        break;

#define rpcdef_invoke_fire_impl(interface, methods) \
    switch (componentId) { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_fire, interface, methods) \
        default: \
            status = Status::INTERFACE_ERROR; \
            break; \
    }

#define rpcdef_case_invoke_broadcast(s, interface, brdcst) \
    case ::rpc::componentId(Broadcast<interface>::brdcst{}): \
        decode(this->brdcst, in.bytes, in.size, status); \
        if (!hasError(status)) { \
            client.onBroadcast(this->brdcst); \
        } \
        break;

#define rpcdef_invoke_broadcast_impl(interface, broadcasts) \
    switch (componentId) { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_case_invoke_broadcast, interface, broadcasts) \
        default: \
            status = Status::INTERFACE_ERROR; \
    }

#define rpcdef_decl_method_input_object(s, interface, method) MethodIn<interface>::method method;

#define RPCDEF_MethodInUnion(interface, methods) \
    template <> \
    union MethodInUnion<interface> { \
        template <class T> \
        void invoke (T& server, \
                uint32_t componentId, \
                barobo_rpc_Request_Fire_payload_t& in, \
                barobo_rpc_Reply_Result_payload_t& out, \
                Status& status) { \
            (void)AssertServerImplementsInterface<T, interface>(); \
            rpcdef_invoke_fire_impl(interface, methods) \
        } \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_method_input_object, interface, methods) \
    };

#define rpcdef_decl_broadcast_object(s, interface, broadcast) Broadcast<interface>::broadcast broadcast;

#define RPCDEF_BroadcastUnion(interface, broadcasts) \
    template <> \
    union BroadcastUnion<interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_decl_broadcast_object, interface, broadcasts) \
        template <class T> \
        void invoke (T& client, \
                uint32_t componentId, \
                barobo_rpc_Broadcast_payload_t& in, \
                Status& status) { \
            (void)AssertClientImplementsInterface<T, interface>(); \
            rpcdef_invoke_broadcast_impl(interface, broadcasts) \
        } \
    };


//////////////////////////////////////////////////////////////////////////////
// Component IDs

#define rpcdef_define_componentId(type, interface, component) \
    template <> \
    constexpr uint32_t componentId (type<interface>::component) { \
        return hash(BOOST_PP_STRINGIZE(component)); \
    }

#define rpcdef_method_componentId(s, interface, method) \
    rpcdef_define_componentId(MethodIn, interface, method)

#define rpcdef_broadcast_componentId(s, interface, broadcast) \
    rpcdef_define_componentId(Broadcast, interface, broadcast)

#define RPCDEF_componentId(interface, methods, broadcasts) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_method_componentId, interface, methods) \
    BOOST_PP_SEQ_FOR_EACH(rpcdef_broadcast_componentId, interface, broadcasts)

//////////////////////////////////////////////////////////////////////////////
// Compile-time assertions that a client implements an interface

#define rpcdef_client_broadcast_assertion(s, interface, broadcast) \
    static_assert(HasMemberFunctionOverloadonBroadcast \
            < T \
            , void(rpc::Broadcast<interface>::broadcast)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " client must implement onBroadcast(" \
            BOOST_PP_STRINGIZE(broadcast) ")");

#define RPCDEF_AssertClientImplementsInterface(interface, broadcasts) \
    template <class T> \
    struct AssertClientImplementsInterface<T, interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_client_broadcast_assertion, interface, broadcasts) \
    };

//////////////////////////////////////////////////////////////////////////////
// Compile-time assertions that a server implements an interface

#define rpcdef_server_fire_assertion(s, interface, method) \
    static_assert(HasMemberFunctionOverloadonFire \
            < T \
            , rpc::MethodResult<interface>::method(rpc::MethodIn<interface>::method)>::value, \
            BOOST_PP_STRINGIZE(interface) \
            " server does not implement onFire(" \
            BOOST_PP_STRINGIZE(method) ")");

#define RPCDEF_AssertServerImplementsInterface(interface, methods) \
    template <class T> \
    struct AssertServerImplementsInterface<T, interface> { \
        BOOST_PP_SEQ_FOR_EACH(rpcdef_server_fire_assertion, interface, methods) \
    };

//////////////////////////////////////////////////////////////////////////////
// Complete header and cpp file defines

#define RPCDEF_CPP(interfaceNames, methods, broadcasts) \
    namespace rpc { \
    RPCDEF_pbFields_methods(rpcdef_underscored_token(interfaceNames), methods) \
    RPCDEF_pbFields_broadcasts(rpcdef_underscored_token(interfaceNames), broadcasts) \
    }

#define RPCDEF_HPP(interfaceNames, version, methods, broadcasts) \
    RPCDEF_FWD_DECL_INTERFACE(interfaceNames) \
    namespace rpc { \
    RPCDEF_Version(rpcdef_cat_scope(interfaceNames), version) \
    RPCDEF_MethodIn(interfaceNames, methods) \
    RPCDEF_MethodResult(interfaceNames, methods) \
    RPCDEF_ResultOf(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_IsMethod(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_Broadcast(interfaceNames, broadcasts) \
    RPCDEF_IsBroadcast(rpcdef_cat_scope(interfaceNames), broadcasts) \
    RPCDEF_componentId(rpcdef_cat_scope(interfaceNames), methods, broadcasts) \
    RPCDEF_AssertServerImplementsInterface(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_AssertClientImplementsInterface(rpcdef_cat_scope(interfaceNames), broadcasts) \
    RPCDEF_MethodInUnion(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_BroadcastUnion(rpcdef_cat_scope(interfaceNames), broadcasts) \
    }

#endif
