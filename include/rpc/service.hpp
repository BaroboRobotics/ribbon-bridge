#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

#include "rpc/stdlibheaders.hpp"
#include "rpc/enableif.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/buffer.hpp"
#include "rpc/message.hpp"
#include "rpc/status.hpp"
#include "rpc/checkversion.hpp"

namespace rpc {

template <class T, class Interface>
class Service {
public:
    using BufferType = Buffer<256>;

    Service () { (void)AssertServiceImplementsInterface<T, Interface>(); }

    template <class Attribute>
    Attribute get (Attribute args, ONLY_IF(IsAttribute<Attribute>::value)) {
        return static_cast<T*>(this)->onGet(args);
    }

    template <class Attribute>
    void set (Attribute args, ONLY_IF(IsSettableAttribute<Attribute>::value)) {
        static_cast<T*>(this)->onSet(args);
    }

    template <class C>
    Status broadcast (C args, ONLY_IF(IsSubscribableAttribute<C>::value || IsBroadcast<C>::value)) {
        if (subscriptionIsActive(mSubscriptions, componentId(C()))) {
            //mSubscriptions.isActive(componentId(C()))) {
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
        }
        return Status::OK;
    }

    template <class Method>
    typename ResultOf<Method>::type fire (Method args, ONLY_IF(IsMethod<Method>::value)) {
        return static_cast<T*>(this)->onFire(args);
    }

    Status refuseConnection (barobo_rpc_Request request) {
        barobo_rpc_Reply reply;
        memset(&reply, 0, sizeof(reply));

        // refactor the next 10 lines or so into a makeConnectionReply(refuse|welcome)
        reply.has_inReplyTo = true;
        reply.inReplyTo = request.id;

        reply.has_connectionReply = true;
        reply.connectionReply.type = barobo_rpc_Reply_ConnectionReply_Type_REFUSAL;
        reply.connectionReply.rpcVersion.major = RPC_VERSION_MAJOR;
        reply.connectionReply.rpcVersion.minor = RPC_VERSION_MINOR;
        reply.connectionReply.rpcVersion.patch = RPC_VERSION_PATCH;
        reply.connectionReply.interfaceVersion.major = Version<Interface>::major;
        reply.connectionReply.interfaceVersion.minor = Version<Interface>::minor;
        reply.connectionReply.interfaceVersion.patch = Version<Interface>::patch;

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

            case barobo_rpc_Request_Type_GET:
                reply.status.value = decltype(reply.status.value)(invokeGet(*this, argument.result, request.get.id, reply.result.payload));
                if (barobo_rpc_Status_OK != reply.status.value) {
                    reply.type = barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                }
                else {
                    reply.type = barobo_rpc_Reply_Type_RESULT;
                    reply.has_result = true;
                    reply.result.id = request.get.id;
                }
                break;
            case barobo_rpc_Request_Type_SET:
                reply.type = barobo_rpc_Reply_Type_STATUS;
                reply.has_status = true;
                reply.status.value = decltype(reply.status.value)(decodeSetPayload(argument.in, request.set.id, request.set.payload));
                if (barobo_rpc_Status_OK == reply.status.value) {
                    reply.status.value = decltype(reply.status.value)(invokeSet(*this, argument.in, request.set.id));
                }
                break;
            case barobo_rpc_Request_Type_FIRE:
                reply.status.value = decltype(reply.status.value)(decodeFirePayload(argument.in, request.fire.id, request.fire.payload));
                if (barobo_rpc_Status_OK != reply.status.value) {
                    reply.type = barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                }
                else {
                    reply.status.value = decltype(reply.status.value)(invokeFire(*this, argument.in, request.fire.id, reply.result.payload));
                    if (barobo_rpc_Status_OK != reply.status.value) {
                        reply.type = barobo_rpc_Reply_Type_STATUS;
                        reply.has_status = true;
                    }
                    else {
                        reply.type = barobo_rpc_Reply_Type_RESULT;
                        reply.has_result = true;
                        reply.result.id = request.fire.id;
                    }
                }
                break;
            case barobo_rpc_Request_Type_SUBSCRIBE:
                reply.type = barobo_rpc_Reply_Type_STATUS;
                reply.has_status = true;
                reply.status.value = decltype(reply.status.value)(activateSubscription(mSubscriptions, request.subscribe.id));
                //mSubscriptions.activate(request.subscribe.id));
                break;
            case barobo_rpc_Request_Type_UNSUBSCRIBE:
                reply.type = barobo_rpc_Reply_Type_STATUS;
                reply.has_status = true;
                reply.status.value = decltype(reply.status.value)(deactivateSubscription(mSubscriptions, request.unsubscribe.id));
                //mSubscriptions.deactivate(request.unsubscribe.id));
                break;
            case barobo_rpc_Request_Type_CONNECT:
                reply.type = barobo_rpc_Reply_Type_CONNECTIONREPLY;
                reply.has_connectionReply = true;
                reply.connectionReply.type = barobo_rpc_Reply_ConnectionReply_Type_WELCOME;
                reply.connectionReply.rpcVersion.major = RPC_VERSION_MAJOR;
                reply.connectionReply.rpcVersion.minor = RPC_VERSION_MINOR;
                reply.connectionReply.rpcVersion.patch = RPC_VERSION_PATCH;
                reply.connectionReply.interfaceVersion.major = Version<Interface>::major;
                reply.connectionReply.interfaceVersion.minor = Version<Interface>::minor;
                reply.connectionReply.interfaceVersion.patch = Version<Interface>::patch;
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

private:
    Subscriptions<Interface> mSubscriptions;
};

} // namespace rpc

#endif
