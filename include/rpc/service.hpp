#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

#include "rpc/enableif.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/buffer.hpp"
#include "rpc/message.hpp"
#include "rpc/error.hpp"
#include "rpc/checkversion.hpp"

namespace rpc {

template <class T, class Interface>
class Service {
    /* TODO: static_assert that T implements Is.... */
public:
    using BufferType = Buffer<256>;

    template <class Attribute>
    Attribute get (Attribute args, ONLY_IF(IsAttribute<Attribute>::value)) {
        return static_cast<T*>(this)->get(args);
    }

    template <class Attribute>
    void set (Attribute args, ONLY_IF(IsAttribute<Attribute>::value)) {
        static_cast<T*>(this)->set(args);
    }

    template <class C>
    void broadcast (C args, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        if (mSubscriptions.isActive(componentId(args))) {
            BufferType buffer;
            buffer.size = sizeof(buffer.bytes);
            auto error = makeBroadcast(
                    buffer.bytes, buffer.size,
                    componentId(args),
                    pbFields(args),
                    &args);
            if (hasError(error)) {
                printf("broadcast encoding failed\n");
                return static_cast<T*>(this)->finalize(error);

            }
            return static_cast<T*>(this)->finalize(buffer);
        }
    }

    template <class Method>
    typename ResultOf<Method>::type fire (Method args, ONLY_IF(IsMethod<Method>::value)) {
        return static_cast<T*>(this)->fire(args);
    }

    Status deliver (BufferType in, BufferType& out) {
        com_barobo_rpc_Request request;
        com_barobo_rpc_Reply reply;
        memset(&reply, 0, sizeof(reply));

        auto err = decode(request, in.bytes, in.size);
        if (hasError(err)) {
            reply.type = com_barobo_rpc_Reply_Type_STATUS;
            reply.has_status = true;
            reply.status.value = com_barobo_rpc_Status_DECODING_FAILURE;
            reply.has_inReplyTo = false;
        }
        else {
            reply.has_inReplyTo = true;
            reply.inReplyTo = request.id;

            switch (request.type) {
                ComponentInUnion<Interface> argument;

                case com_barobo_rpc_Request_Type_GET:
                    reply.status.value = decltype(reply.status.value)(invokeGet(*this, argument, request.get.id, reply.output.payload));
                    if (com_barobo_rpc_Status_OK != reply.status.value) {
                        reply.type = com_barobo_rpc_Reply_Type_STATUS;
                        reply.has_status = true;
                    }
                    else {
                        reply.type = com_barobo_rpc_Reply_Type_OUTPUT;
                        reply.has_output = true;
                        reply.output.id = request.get.id;
                    }
                    break;
                case com_barobo_rpc_Request_Type_SET:
                    reply.type = com_barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    reply.status.value = decltype(reply.status.value)(decodeSetPayload(argument, request.set.id, request.set.payload));
                    if (com_barobo_rpc_Status_OK == reply.status.value) {
                        reply.status.value = decltype(reply.status.value)(invokeSet(*this, argument, request.set.id));
                    }
                    break;
                case com_barobo_rpc_Request_Type_FIRE:
                    reply.status.value = decltype(reply.status.value)(decodeFirePayload(argument, request.fire.id, request.fire.payload));
                    if (com_barobo_rpc_Status_OK != reply.status.value) {
                        reply.type = com_barobo_rpc_Reply_Type_STATUS;
                        reply.has_status = true;
                    }
                    else {
                        reply.status.value = decltype(reply.status.value)(invokeFire(*this, argument, request.fire.id, reply.output.payload));
                        if (reply.output.payload.size) {
                            reply.type = com_barobo_rpc_Reply_Type_OUTPUT;
                            reply.has_output = true;
                            reply.output.id = request.fire.id;
                        }
                        else {
                            reply.type = com_barobo_rpc_Reply_Type_STATUS;
                            reply.has_status = true;
                        }
                    }
                    break;
                case com_barobo_rpc_Request_Type_SUBSCRIBE:
                    reply.type = com_barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    reply.status.value = decltype(reply.status.value)(mSubscriptions.activate(request.subscribe.id));
                    break;
                case com_barobo_rpc_Request_Type_UNSUBSCRIBE:
                    reply.type = com_barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    reply.status.value = decltype(reply.status.value)(mSubscriptions.deactivate(request.subscribe.id));
                    break;
                case com_barobo_rpc_Request_Type_RESET:
                    mSubscriptions.reset();
                    reply.type = com_barobo_rpc_Reply_Type_VERSION;
                    reply.has_version = true;
                    reply.version.rpc.major = RPC_VERSION_MAJOR;
                    reply.version.rpc.minor = RPC_VERSION_MINOR;
                    reply.version.rpc.patch = RPC_VERSION_PATCH;
                    reply.version.interface.major = Version<Interface>::major;
                    reply.version.interface.minor = Version<Interface>::minor;
                    reply.version.interface.patch = Version<Interface>::patch;
                    break;
                default:
                    reply.type = com_barobo_rpc_Reply_Type_STATUS;
                    reply.has_status = true;
                    reply.status.value = com_barobo_rpc_Status_ILLEGAL_OPERATION;
                    break;
            }
        }

        out.size = sizeof(out.bytes);
        return encode(reply, out.bytes, out.size, out.size);
    }

private:
    Subscriptions<Interface> mSubscriptions;
};

} // namespace rpc

#endif
