#ifndef RPC_PRINTMESSAGE_HPP
#define RPC_PRINTMESSAGE_HPP

#include "rpc.pb.h"

namespace rpc {

const char* toObjectTypeToString (com_barobo_rpc_ToObject_Type type);
void printToObject (const com_barobo_rpc_ToObject& toObject);
void printFromObject (const com_barobo_rpc_FromObject& message);
void printReply (const com_barobo_rpc_Reply& message);
void printMessage (const com_barobo_rpc_Message& message);

} // namespace rpc

#endif
