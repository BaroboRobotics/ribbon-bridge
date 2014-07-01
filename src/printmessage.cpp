#include "rpc/stdlibheaders.hpp"
#include "rpc/printmessage.hpp"

namespace rpc {

void printRequestComponentInvocation (const com_barobo_rpc_Request_Component_Invocation& invocation) {
    if (invocation.payload.size) {
        printf("{");
        for (size_t i = 0; i < invocation.payload.size; ++i) {
            printf(" %x", int(invocation.payload.bytes[i]));
        }
    }
    else {
        printf("(empty)");
    }

    printf(" }\n");
}

void printRequestComponentSubscription (const com_barobo_rpc_Request_Component_Subscription& subscription) {
    if (com_barobo_rpc_Request_Component_Subscription_Type_SUBSCRIBE == subscription.type) {
        printf("SUBSCRIBE\n");
    }
    else {
        printf("UNSUBSCRIBE\n");
    }
}

void printRequestComponent (const com_barobo_rpc_Request_Component& component) {
    printf("{\n");
    printf("\t\tid : %" PRIu32 "\n", component.id);

    if (component.has_invocation) {
        printf("\t\tinvocation : ");
        printRequestComponentInvocation(component.invocation);
    }

    if (component.has_subscription) {
        printf("\t\tsubscription : ");
        printRequestComponentSubscription(component.subscription);
    }
}

void printVersion (const com_barobo_rpc_Version& version) {
    printf("{\n"
           "\t\t\tmajor : %" PRIu32 "\n"
           "\t\t\tminor : %" PRIu32 "\n"
           "\t\t\tpatch : %" PRIu32 "\n"
           "\t\t}\n",
           version.major,
           version.minor,
           version.patch);
}

void printRequestStatus (const com_barobo_rpc_Request_Status& status) {
    printf("\tStatus {\n");
    printf("\t\trpcVersion : {\n");
    printf("\t\t\tmajor : %" PRIu32 "\n"
           "\t\t\tminor : %" PRIu32 "\n"
           "\t\t\tpatch : %" PRIu32 "\n"
           "\t\t}\n",
           status.rpcVersion.major,
           status.rpcVersion.minor,
           status.rpcVersion.patch);
    printf("\t\trpcVersion : ");
    printVersion(status.rpcVersion);
    printf("\t\tinterfaceVersion : ");
    printVersion(status.interfaceVersion);
}

void printRequest (const com_barobo_rpc_Request& request) {
    printf("Request {\n");
    printf("\tid : %" PRIu32 "\n", request.id);

    if (request.has_component) {
        printf("\tcomponent : ");
        printRequestComponent(request.component);
    }

    if (request.has_status) {
        printf("\tstatus : ");
        printRequestStatus(request.status);
    }
}

} // namespace rpc
