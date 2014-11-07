#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

#include "rpc/stdlibheaders.hpp"
#include "rpc/enableif.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/buffer.hpp"
#include "rpc/message.hpp"
#include "rpc/status.hpp"
#include "rpc/version.hpp"

namespace rpc {

template <class T, class Interface>
class Service {
public:
    using BufferType = Buffer<256>;

    Service () { (void)AssertServiceImplementsInterface<T, Interface>(); }

    template <class C>
    Status broadcast (C args, ONLY_IF(IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto status = makeBroadcast(
                buffer.bytes, buffer.size,
                componentId(C()),
                pbFields(args),
                &args);
        if (hasError(status)) {
            return status;
        }
        static_cast<T*>(this)->bufferToProxy(buffer);
        return Status::OK;
    }

    template <class Method>
    typename ResultOf<Method>::type fire (Method args, ONLY_IF(IsMethod<Method>::value)) {
        return static_cast<T*>(this)->onFire(args);
    }

    Status refuseConnection (barobo_rpc_ClientMessage clMessage) {
        barobo_rpc_ServerMessage svMessage;
        memset(&svMessage, 0, sizeof(svMessage));

        // refactor the next 10 lines or so into a makeServiceInfo(refuse|welcome)
        svMessage.type = barobo_rpc_ServerMessage_Type_REPLY;
        svMessage.has_reply = true;
        svMessage.reply.type = barobo_rpc_Reply_Type_SERVICEINFO;
        svMessage.reply.has_serviceInfo = true;
        svMessage.reply.serviceInfo.type = barobo_rpc_Reply_ServiceInfo_Type_REFUSAL;
        svMessage.reply.serviceInfo.rpcVersion.major = Version<>::major;
        svMessage.reply.serviceInfo.rpcVersion.minor = Version<>::minor;
        svMessage.reply.serviceInfo.rpcVersion.patch = Version<>::patch;
        svMessage.reply.serviceInfo.interfaceVersion.major = Version<Interface>::major;
        svMessage.reply.serviceInfo.interfaceVersion.minor = Version<Interface>::minor;
        svMessage.reply.serviceInfo.interfaceVersion.patch = Version<Interface>::patch;
        svMessage.has_inReplyTo = true;
        svMessage.inReplyTo = clMessage.id;

        BufferType response;
        response.size = sizeof(response.bytes);
        Status status;
        encode(svMessage, response.bytes, response.size, response.size, status);
        if (!hasError(status)) {
            static_cast<T*>(this)->bufferToProxy(response);
        }

        return status;
    }

    Status refuseRequest (barobo_rpc_ClientMessage clMessage) {
        barobo_rpc_ServerMessage svMessage;
        memset(&svMessage, 0, sizeof(svMessage));

        svMessage.type = barobo_rpc_ServerMessage_Type_REPLY;
        svMessage.has_reply = true;
        svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
        svMessage.reply.has_status = true;
        svMessage.reply.status.value = barobo_rpc_Status_NOT_CONNECTED;
        svMessage.has_inReplyTo = true;
        svMessage.inReplyTo = clMessage.id;

        BufferType response;
        response.size = sizeof(response.bytes);
        Status status;
        encode(svMessage, response.bytes, response.size, response.size, status);
        if (!hasError(status)) {
            static_cast<T*>(this)->bufferToProxy(response);
        }

        return status;
    }

    Status receiveProxyBuffer (BufferType in) {
        barobo_rpc_ClientMessage message;
        Status status;
        decode(message, in.bytes, in.size, status);
        if (hasError(status)) {
            return status;
        }
        return receiveProxyRequest(message);
    }

    Status receiveProxyRequest (barobo_rpc_ClientMessage clMessage) {
        barobo_rpc_ServerMessage svMessage;
        memset(&svMessage, 0, sizeof(svMessage));

        svMessage.type = barobo_rpc_ServerMessage_Type_REPLY;
        svMessage.has_inReplyTo = true;
        svMessage.inReplyTo = clMessage.id;

        svMessage.has_reply = true;
        switch (clMessage.request.type) {
            union {
                ComponentInUnion<Interface> in;
                ComponentResultUnion<Interface> result;
            } argument;

            case barobo_rpc_Request_Type_CONNECT:
                svMessage.reply.type = barobo_rpc_Reply_Type_SERVICEINFO;
                svMessage.reply.has_serviceInfo = true;
                svMessage.reply.serviceInfo.type = barobo_rpc_Reply_ServiceInfo_Type_WELCOME;
                svMessage.reply.serviceInfo.rpcVersion.major = Version<>::major;
                svMessage.reply.serviceInfo.rpcVersion.minor = Version<>::minor;
                svMessage.reply.serviceInfo.rpcVersion.patch = Version<>::patch;
                svMessage.reply.serviceInfo.interfaceVersion.major = Version<Interface>::major;
                svMessage.reply.serviceInfo.interfaceVersion.minor = Version<Interface>::minor;
                svMessage.reply.serviceInfo.interfaceVersion.patch = Version<Interface>::patch;
                break;
            case barobo_rpc_Request_Type_DISCONNECT:
                svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                svMessage.reply.has_status = true;
                svMessage.reply.status.value = barobo_rpc_Status_OK;
                break;
            case barobo_rpc_Request_Type_FIRE:
                if (!clMessage.request.has_fire) {
                    svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                    svMessage.reply.has_status = true;
                    svMessage.reply.status.value = barobo_rpc_Status_INCONSISTENT_REQUEST;
                    break;
                }
                svMessage.reply.status.value = decltype(svMessage.reply.status.value)(decodeFirePayload(argument.in, clMessage.request.fire.id, clMessage.request.fire.payload));
                if (barobo_rpc_Status_OK != svMessage.reply.status.value) {
                    svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                    svMessage.reply.has_status = true;
                    break;
                }
                svMessage.reply.status.value = decltype(svMessage.reply.status.value)(invokeFire(*this, argument.in, clMessage.request.fire.id, svMessage.reply.result.payload));
                if (barobo_rpc_Status_OK != svMessage.reply.status.value) {
                    svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                    svMessage.reply.has_status = true;
                    break;
                }
                svMessage.reply.type = barobo_rpc_Reply_Type_RESULT;
                svMessage.reply.has_result = true;
                svMessage.reply.result.id = clMessage.request.fire.id;
                break;
            default:
                svMessage.reply.type = barobo_rpc_Reply_Type_STATUS;
                svMessage.reply.has_status = true;
                svMessage.reply.status.value = barobo_rpc_Status_ILLEGAL_OPERATION;
                break;
        }

        BufferType response;
        response.size = sizeof(response.bytes);
        Status status;
        encode(svMessage, response.bytes, response.size, response.size, status);
        if (!hasError(status)) {
            static_cast<T*>(this)->bufferToProxy(response);
        }

        return status;
    }
};

} // namespace rpc

#endif
