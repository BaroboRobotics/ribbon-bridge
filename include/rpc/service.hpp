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

    Status refuseConnection (barobo_rpc_Request request) {
        barobo_rpc_Reply reply;
        memset(&reply, 0, sizeof(reply));

        // refactor the next 10 lines or so into a makeServiceInfo(refuse|welcome)
        reply.has_inReplyTo = true;
        reply.inReplyTo = request.id;

        reply.type = barobo_rpc_Reply_Type_SERVICEINFO;
        reply.has_serviceInfo = true;
        reply.serviceInfo.type = barobo_rpc_Reply_ServiceInfo_Type_REFUSAL;
        reply.serviceInfo.rpcVersion.major = Version<>::major;
        reply.serviceInfo.rpcVersion.minor = Version<>::minor;
        reply.serviceInfo.rpcVersion.patch = Version<>::patch;
        reply.serviceInfo.interfaceVersion.major = Version<Interface>::major;
        reply.serviceInfo.interfaceVersion.minor = Version<Interface>::minor;
        reply.serviceInfo.interfaceVersion.patch = Version<Interface>::patch;

        BufferType response;
        response.size = sizeof(response.bytes);
        auto status = encode(reply, response.bytes, response.size, response.size);
        if (!hasError(status)) {
            static_cast<T*>(this)->bufferToProxy(response);
        }

        return status;
    }

    Status refuseRequest (barobo_rpc_Request request) {
        barobo_rpc_Reply reply;
        memset(&reply, 0, sizeof(reply));

        reply.has_inReplyTo = true;
        reply.inReplyTo = request.id;

        reply.type = barobo_rpc_Reply_Type_STATUS;
        reply.has_status = true;
        reply.status.value = barobo_rpc_Status_NOT_CONNECTED;

        BufferType response;
        response.size = sizeof(response.bytes);
        auto status = encode(reply, response.bytes, response.size, response.size);
        if (!hasError(status)) {
            static_cast<T*>(this)->bufferToProxy(response);
        }

        return status;
    }

    Status receiveProxyBuffer (BufferType in) {
        barobo_rpc_Request request;
        auto err = decode(request, in.bytes, in.size);
        if (hasError(err)) {
            return err;
        }
        return receiveProxyRequest(request);
    }

    Status receiveProxyRequest (barobo_rpc_Request request) {
        barobo_rpc_Reply reply;
        memset(&reply, 0, sizeof(reply));

        reply.has_inReplyTo = true;
        reply.inReplyTo = request.id;

        switch (request.type) {
            union {
                ComponentInUnion<Interface> in;
                ComponentResultUnion<Interface> result;
            } argument;

            case barobo_rpc_Request_Type_CONNECT:
                reply.type = barobo_rpc_Reply_Type_SERVICEINFO;
                reply.has_serviceInfo = true;
                reply.serviceInfo.type = barobo_rpc_Reply_ServiceInfo_Type_WELCOME;
                reply.serviceInfo.rpcVersion.major = Version<>::major;
                reply.serviceInfo.rpcVersion.minor = Version<>::minor;
                reply.serviceInfo.rpcVersion.patch = Version<>::patch;
                reply.serviceInfo.interfaceVersion.major = Version<Interface>::major;
                reply.serviceInfo.interfaceVersion.minor = Version<Interface>::minor;
                reply.serviceInfo.interfaceVersion.patch = Version<Interface>::patch;
                break;
            case barobo_rpc_Request_Type_DISCONNECT:
                reply.type = barobo_rpc_Reply_Type_STATUS;
                reply.has_status = true;
                reply.status.value = barobo_rpc_Status_OK;
                break;
            case barobo_rpc_Request_Type_FIRE:
                if (!request.has_fire) {
                    reply.type = barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    reply.status.value = barobo_rpc_Status_INCONSISTENT_REQUEST;
                    break;
                }
                reply.status.value = decltype(reply.status.value)(decodeFirePayload(argument.in, request.fire.id, request.fire.payload));
                if (barobo_rpc_Status_OK != reply.status.value) {
                    reply.type = barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    break;
                }
                reply.status.value = decltype(reply.status.value)(invokeFire(*this, argument.in, request.fire.id, reply.result.payload));
                if (barobo_rpc_Status_OK != reply.status.value) {
                    reply.type = barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    break;
                }
                reply.type = barobo_rpc_Reply_Type_RESULT;
                reply.has_result = true;
                reply.result.id = request.fire.id;
                break;
            default:
                reply.type = barobo_rpc_Reply_Type_STATUS;
                reply.has_status = true;
                reply.status.value = barobo_rpc_Status_ILLEGAL_OPERATION;
                break;
        }

        BufferType response;
        response.size = sizeof(response.bytes);
        auto status = encode(reply, response.bytes, response.size, response.size);
        if (!hasError(status)) {
            static_cast<T*>(this)->bufferToProxy(response);
        }

        return status;
    }
};

} // namespace rpc

#endif
