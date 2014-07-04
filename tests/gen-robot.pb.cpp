#include "rpc/stdlibheaders.hpp"
#include "gen-robot.pb.hpp"
#include "rpc/message.hpp"
#include "rpc/service.hpp"

namespace rpc {

template <>
const pb_field_t* pbFields (com_barobo_Robot_move_In) {
    return com_barobo_Robot_move_In_fields;
}

template <>
bool isAttribute<com::barobo::Robot> (uint32_t id) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (id) {
        case Id::motorPower:
            return true;
        default:
            return false;
    }
}

template <>
bool isMethod<com::barobo::Robot> (uint32_t id) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (id) {
        case Id::move:
            return true;
        default:
            return false;
    }
}

template <>
bool isBroadcast<com::barobo::Robot> (uint32_t id) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (id) {
        case Id::buttonPress:
            return true;
        default:
            return false;
    }
}

template <>
Error decodeSetPayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        const com_barobo_rpc_Request_Set_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of non-read-only attributes
        case Id::motorPower:
            return decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, payload.bytes, payload.size);
        default:
            if (isAttribute(componentId)) {
                return Error::READ_ONLY;
            }
            return isComponent(componentId) ?
                Error::ILLEGAL_OPERATION :
                Error::NO_SUCH_COMPONENT;
    }
}

template <>
Error decodeFirePayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        const com_barobo_rpc_Request_Fire_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of methods
        case Id::move:
            return decodeProtobuf(&args.move.in, com_barobo_Robot_move_In_fields, payload.bytes, payload.size);
        default:
            return isComponent(componentId) ?
                Error::ILLEGAL_OPERATION :
                Error::NO_SUCH_COMPONENT;
    }
}

template <>
Error decodeBroadcastPayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        const com_barobo_rpc_Request_Broadcast_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of subscribable attributes
        case Id::motorPower:
            return decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, payload.bytes, payload.size);
        // list of broadcasts
        case Id::buttonPress:
            return decodeProtobuf(&args.buttonPress, com_barobo_Robot_buttonPress_fields, payload.bytes, payload.size);
        default:
            if (isAttribute(componentId)) {
                return Error::NO_SUBSCRIPTIONS;
            }
            return isComponent(componentId) ?
                Error::ILLEGAL_OPERATION :
                Error::NO_SUCH_COMPONENT;
    }
}

template <>
Error decodeOutputErrorPayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        const com_barobo_rpc_Reply_Output_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of methods
        case Id::move:
            return decodeProtobuf(&args.move.error, com_barobo_Robot_move_Error_fields, payload.bytes, payload.size);
        default:
            return isComponent(componentId) ?
                Error::ILLEGAL_OPERATION :
                Error::NO_SUCH_COMPONENT;
    }
}

template <>
Error decodeOutputOutPayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        const com_barobo_rpc_Reply_Output_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of methods
        case Id::move:
            return decodeProtobuf(&args.move.out, com_barobo_Robot_move_Out_fields, payload.bytes, payload.size);
        default:
            return isComponent(componentId) ?
                Error::ILLEGAL_OPERATION :
                Error::NO_SUCH_COMPONENT;
    }
}

} // namespace rpc
