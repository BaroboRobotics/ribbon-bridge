#ifndef RPC_DEF_HPP
#define RPC_DEF_HPP

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/transform.hpp>

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

#endif
