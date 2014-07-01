#ifndef RPC_PRINTMESSAGE_HPP
#define RPC_PRINTMESSAGE_HPP

#include "rpc.pb.h"

namespace rpc {

void printRequestComponentInvocation (const com_barobo_rpc_Request_Component_Invocation& invocation);
void printRequestComponentSubscription (const com_barobo_rpc_Request_Component_Subscription& subscription);
void printRequestComponent (const com_barobo_rpc_Request_Component& component);
void printVersion (const com_barobo_rpc_Version& version);
void printRequestStatus (const com_barobo_rpc_Request_Status& status);
void printRequest (const com_barobo_rpc_Request& request);

} // namespace rpc

#endif
