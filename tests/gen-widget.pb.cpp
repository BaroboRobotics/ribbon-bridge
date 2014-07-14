#include "rpc/stdlibheaders.hpp"
#include "gen-widget.pb.hpp"
#include "rpc/message.hpp"

namespace rpc {

template <>
const pb_field_t* pbFields (barobo_Widget_attribute) {
    return barobo_Widget_attribute_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_readonlyAttribute) {
    return barobo_Widget_readonlyAttribute_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_noSubscriptionsAttribute) {
    return barobo_Widget_noSubscriptionsAttribute_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_readonlyNoSubscriptionsAttribute) {
    return barobo_Widget_readonlyNoSubscriptionsAttribute_fields;
}

template <>
const pb_field_t* pbFields (barobo_Widget_nullaryNoResult_In) {
    return barobo_Widget_nullaryNoResult_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryWithResultOut_In) {
    return barobo_Widget_nullaryWithResultOut_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryWithResultError_In) {
    return barobo_Widget_nullaryWithResultError_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryWithResult_In) {
    return barobo_Widget_nullaryWithResult_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryNoResult_In) {
    return barobo_Widget_unaryNoResult_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryWithResultOut_In) {
    return barobo_Widget_unaryWithResultOut_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryWithResultError_In) {
    return barobo_Widget_unaryWithResultError_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryWithResult_In) {
    return barobo_Widget_unaryWithResult_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryNoResult_Result) {
    return barobo_Widget_nullaryNoResult_Result_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryWithResultOut_Result) {
    return barobo_Widget_nullaryWithResultOut_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryWithResultError_Result) {
    return barobo_Widget_nullaryWithResultError_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_nullaryWithResult_Result) {
    return barobo_Widget_nullaryWithResult_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryNoResult_Result) {
    return barobo_Widget_unaryNoResult_Result_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryWithResultOut_Result) {
    return barobo_Widget_unaryWithResultOut_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryWithResultError_Result) {
    return barobo_Widget_unaryWithResultError_In_fields;
}
template <>
const pb_field_t* pbFields (barobo_Widget_unaryWithResult_Result) {
    return barobo_Widget_unaryWithResult_In_fields;
}

template <>
const pb_field_t* pbFields (barobo_Widget_broadcast) {
    return barobo_Widget_broadcast_fields;
}

template <>
bool isAttribute<barobo::Widget> (uint32_t id) {
    using Id = ComponentId<barobo::Widget>;
    switch (id) {
        case Id::attribute:
        case Id::readonlyAttribute:
        case Id::noSubscriptionsAttribute:
        case Id::readonlyNoSubscriptionsAttribute:
            return true;
        default:
            return false;
    }
}

template <>
bool isMethod<barobo::Widget> (uint32_t id) {
    using Id = ComponentId<barobo::Widget>;
    switch (id) {
        case Id::nullaryNoResult:
        case Id::nullaryWithResultOut:
        case Id::nullaryWithResultError:
        case Id::nullaryWithResult:
        case Id::unaryNoResult:
        case Id::unaryWithResultOut:
        case Id::unaryWithResultError:
        case Id::unaryWithResult:
            return true;
        default:
            return false;
    }
}

template <>
bool isBroadcast<barobo::Widget> (uint32_t id) {
    using Id = ComponentId<barobo::Widget>;
    switch (id) {
        case Id::broadcast:
            return true;
        default:
            return false;
    }
}

template <>
Status decodeSetPayload (ComponentInUnion<barobo::Widget>& args,
        uint32_t componentId,
        barobo_rpc_Request_Set_payload_t& payload) {
    using Id = ComponentId<barobo::Widget>;
    switch (componentId) {
        // list of non-read-only attributes
        case Id::attribute:
            return decode(args.attribute, payload.bytes, payload.size);
#if 0
        case Id::readonlyAttribute:
            return decode(args.readonlyAttribute, payload.bytes, payload.size);
#endif
        case Id::noSubscriptionsAttribute:
            return decode(args.attribute, payload.bytes, payload.size);
#if 0
        case Id::readonlyNoSubscriptionsAttribute:
            return decode(args.readonlyNoSubscriptionsAttribute, payload.bytes, payload.size);
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

template <>
Status decodeFirePayload (ComponentInUnion<barobo::Widget>& args,
        uint32_t componentId,
        barobo_rpc_Request_Fire_payload_t& payload) {
    using Id = ComponentId<barobo::Widget>;
    switch (componentId) {
        // list of methods
        case Id::nullaryNoResult:
            return decode(args.nullaryNoResult, payload.bytes, payload.size);
        case Id::nullaryWithResultOut:
            return decode(args.nullaryWithResultOut, payload.bytes, payload.size);
        case Id::nullaryWithResultError:
            return decode(args.nullaryWithResultError, payload.bytes, payload.size);
        case Id::nullaryWithResult:
            return decode(args.nullaryWithResult, payload.bytes, payload.size);
        case Id::unaryNoResult:
            return decode(args.unaryNoResult, payload.bytes, payload.size);
        case Id::unaryWithResultOut:
            return decode(args.unaryWithResultOut, payload.bytes, payload.size);
        case Id::unaryWithResultError:
            return decode(args.unaryWithResultError, payload.bytes, payload.size);
        case Id::unaryWithResult:
            return decode(args.unaryWithResult, payload.bytes, payload.size);
        default:
            return isComponent<barobo::Widget>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

template <>
Status decodeBroadcastPayload (ComponentResultUnion<barobo::Widget>& args,
        uint32_t componentId,
        barobo_rpc_Reply_Broadcast_payload_t& payload) {
    using Id = ComponentId<barobo::Widget>;
    switch (componentId) {
        // list of subscribable attributes
        case Id::attribute:
            return decode(args.attribute, payload.bytes, payload.size);
        case Id::readonlyAttribute:
            return decode(args.readonlyAttribute, payload.bytes, payload.size);
#if 0
        case Id::noSubscriptionsAttribute:
            return decode(args.noSubscriptionsAttribute, payload.bytes, payload.size);
        case Id::readonlyNoSubscriptionsAttribute:
            return decode(args.readonlyNoSubscriptionsAttribute, payload.bytes, payload.size);
#endif
        // list of broadcasts
        case Id::broadcast:
            return decode(args.broadcast, payload.bytes, payload.size);
        default:
            if (isAttribute<barobo::Widget>(componentId)) {
                return Status::NO_SUBSCRIPTIONS;
            }
            return isComponent<barobo::Widget>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

template <>
Status decodeResultPayload (ComponentResultUnion<barobo::Widget>& args,
        uint32_t componentId,
        barobo_rpc_Reply_Result_payload_t& payload) {
    using Id = ComponentId<barobo::Widget>;
    switch (componentId) {
        // list of attributes
        case Id::attribute:
            return decode(args.attribute, payload.bytes, payload.size);
        case Id::readonlyAttribute:
            return decode(args.readonlyAttribute, payload.bytes, payload.size);
        case Id::noSubscriptionsAttribute:
            return decode(args.noSubscriptionsAttribute, payload.bytes, payload.size);
        case Id::readonlyNoSubscriptionsAttribute:
            return decode(args.readonlyNoSubscriptionsAttribute, payload.bytes, payload.size);
        // list of methods
        case Id::nullaryNoResult:
            return decode(args.nullaryNoResult, payload.bytes, payload.size);
        case Id::nullaryWithResultOut:
            return decode(args.nullaryWithResultOut, payload.bytes, payload.size);
        case Id::nullaryWithResultError:
            return decode(args.nullaryWithResultError, payload.bytes, payload.size);
        case Id::nullaryWithResult:
            return decode(args.nullaryWithResult, payload.bytes, payload.size);
        case Id::unaryNoResult:
            return decode(args.unaryNoResult, payload.bytes, payload.size);
        case Id::unaryWithResultOut:
            return decode(args.unaryWithResultOut, payload.bytes, payload.size);
        case Id::unaryWithResultError:
            return decode(args.unaryWithResultError, payload.bytes, payload.size);
        case Id::unaryWithResult:
            return decode(args.unaryWithResult, payload.bytes, payload.size);
        default:
            return isComponent<barobo::Widget>(componentId) ?
                Status::ILLEGAL_OPERATION :
                Status::NO_SUCH_COMPONENT;
    }
}

} // namespace rpc
