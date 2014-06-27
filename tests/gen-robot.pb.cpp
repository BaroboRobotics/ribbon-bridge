#include "stdlibheaders.hpp"
#include "gen-robot.pb.hpp"
#include "rpc/encode_decode.hpp"
#include "rpc/object.hpp"

namespace rpc {

template <>
void decodeToObjectPayload (ArgumentUnion<com::barobo::Robot>& args, com_barobo_rpc_ToObject& toObject) {
    if (com_barobo_rpc_ToObject_Type_SET == toObject.type ||
            com_barobo_rpc_ToObject_Type_FIRE == toObject.type) {
        switch (toObject.elementId) {
            //////////////////////////////////////////////////////////////////
            // ATTRIBUTES
            case ComponentId<com::barobo::Robot>::motorPower:
                decodeProtobuf(&args.motorPower, com_barobo_Robot_motorPower_fields, toObject.payload.bytes, toObject.payload.size);
                break;

            //////////////////////////////////////////////////////////////////
            // METHODS
            case ComponentId<com::barobo::Robot>::move:
                // Pay special attention here! The payload is ONLY the input
                // parameters, so use <interface>_<method>_In_fields.
                //                                                  vv <-- here
                decodeProtobuf(&args.move.in, com_barobo_Robot_move_In_fields, toObject.payload.bytes, toObject.payload.size);
                break;

            // Broadcast payloads are never serialized in ToObject messages.
            
            default:
                printf("TODO: freak out and return an error when an interface"
                        "component isn't found ...\n");
                break;
        }
    }
}

} // namespace rpc
