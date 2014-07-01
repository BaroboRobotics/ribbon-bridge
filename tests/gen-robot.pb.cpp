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
void decodePayload (ComponentUnion<com::barobo::Robot>& args,
        uint32_t componentId,
        com_barobo_rpc_Request_Component_Invocation& invocation) {
    if (!invocation.payload.size) {
        return;
    }

    switch (componentId) {
        // ATTRIBUTES
        case ComponentId<com::barobo::Robot>::motorPower:
            decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, invocation.payload.bytes, invocation.payload.size);
            break;

        // METHODS
        case ComponentId<com::barobo::Robot>::move:
            // Pay special attention here! The payload is ONLY the input
            // parameters, so use <interface>_<method>_In_fields.
            //                                                  vv <-- here
            decodeProtobuf(&args.move.in, com_barobo_Robot_move_In_fields, invocation.payload.bytes, invocation.payload.size);
            break;

        // Broadcast payloads are never serialized in requests.
        
        default:
            printf("TODO: freak out and return an error when an interface"
                    "component isn't found ...\n");
            break;
    }
}

} // namespace rpc
