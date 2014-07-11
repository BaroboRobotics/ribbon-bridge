#include "rpc/stdlibheaders.hpp"
#include "gen-robot.pb.hpp"
#include "rpc/message.hpp"
#include "rpc/service.hpp"

namespace rpc {

template <>
const pb_field_t* pbFields (com_barobo_Robot_motorPower) {
    return com_barobo_Robot_motorPower_fields;
}

template <>
const pb_field_t* pbFields (com_barobo_Robot_move_In) {
    return com_barobo_Robot_move_In_fields;
}

template <>
const pb_field_t* pbFields (com_barobo_Robot_move_Result) {
    return com_barobo_Robot_move_Result_fields;
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
Status decodeSetPayload (ComponentInUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        com_barobo_rpc_Request_Set_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of non-read-only attributes
        case Id::motorPower:
            return decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, payload.bytes, payload.size);
        default:
            if (isAttribute<com::barobo::Robot>(componentId)) {
                return Status::READ_ONLY;
            }
            return isComponent<com::barobo::Robot>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

template <>
Status decodeFirePayload (ComponentInUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        com_barobo_rpc_Request_Fire_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of methods
        case Id::move:
            return decodeProtobuf(&args.move, com_barobo_Robot_move_In_fields, payload.bytes, payload.size);
        default:
            return isComponent<com::barobo::Robot>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

template <>
Status decodeBroadcastPayload (ComponentResultUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        com_barobo_rpc_Reply_Broadcast_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of subscribable attributes
        case Id::motorPower:
            return decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, payload.bytes, payload.size);
        // list of broadcasts
        case Id::buttonPress:
            return decodeProtobuf(&args.buttonPress, com_barobo_Robot_buttonPress_fields, payload.bytes, payload.size);
        default:
            if (isAttribute<com::barobo::Robot>(componentId)) {
                return Status::NO_SUBSCRIPTIONS;
            }
            return isComponent<com::barobo::Robot>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

template <>
Status decodeResultPayload (ComponentResultUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        com_barobo_rpc_Reply_Result_payload_t& payload) {
    using Id = ComponentId<com::barobo::Robot>;
    switch (componentId) {
        // list of attributes
        case Id::motorPower:
            return decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, payload.bytes, payload.size);
        // list of methods
        case Id::move:
            return decodeProtobuf(&args.move, com_barobo_Robot_move_Result_fields, payload.bytes, payload.size);
        default:
            return isComponent<com::barobo::Robot>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

} // namespace rpc
