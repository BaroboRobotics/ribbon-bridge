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
            if (makeBroadcast(
                    buffer.bytes, buffer.size,
                    componentId(args),
                    pbFields(args),
                    &args)) {
                printf("broadcast encoding failed\n");
                return;
            }
            static_cast<T*>(this)->finalize(buffer);
        }
    }

    template <class Method>
    ResultOf<Method> on (Method args, ONLY_IF(IsMethod<Method>::value)) {
        return static_cast<T*>(this)->on(args);
    }

    Error deliver (BufferType in, BufferType& out) {
        com_barobo_rpc_Request request;
        com_barobo_rpc_Reply reply;
        memset(&reply, 0, sizeof(reply));

        auto err = decode(request, in.bytes, in.size);
        if (hasError(err)) {
            reply.type = com_barobo_rpc_Reply_Type_ERROR;
            reply.has_error = true;
            reply.error.value = com_barobo_rpc_Error_DECODING_FAILURE;
            reply.has_inReplyTo = false;
        }
        else {
            reply.has_inReplyTo = true;
            reply.inReplyTo = request.id;

            printf("Received the following message:\n");
            //printRequest(request);

            switch (request.type) {
                ComponentUnion<Interface> argument;

                case com_barobo_rpc_Request_Type_GET:
                    reply.error.value = decltype(reply.error.value)(invokeGet(*this, argument, request.get.id, reply.output.payload));
                    if (com_barobo_rpc_Error_NO_ERROR != reply.error.value) {
                        reply.type = com_barobo_rpc_Reply_Type_ERROR;
                        reply.has_error = true;
                    }
                    else {
                        reply.type = com_barobo_rpc_Reply_Type_OUTPUT;
                        reply.has_output = true;
                        reply.output.type = com_barobo_rpc_Reply_Output_Type_OUT;
                    }
                    break;
                case com_barobo_rpc_Request_Type_SET:
                    reply.type = com_barobo_rpc_Reply_Type_ERROR;
                    reply.has_error = true;
                    reply.error.value = decltype(reply.error.value)(decodeSetPayload(argument, request.set.id, request.set.payload));
                    if (com_barobo_rpc_Error_NO_ERROR == reply.error.value) {
                        reply.error.value = decltype(reply.error.value)(invokeSet(*this, argument, request.set.id));
                    }
                    break;
                case com_barobo_rpc_Request_Type_FIRE:
                    reply.error.value = decltype(reply.error.value)(decodeFirePayload(argument, request.fire.id, request.fire.payload));
                    if (com_barobo_rpc_Error_NO_ERROR != reply.error.value) {
                        reply.type = com_barobo_rpc_Reply_Type_ERROR;
                        reply.has_error = true;
                    }
                    else {
                        bool isExceptional = false;
                        reply.error.value = decltype(reply.error.value)(invokeFire(*this, argument, request.fire.id, reply.output.payload, isExceptional));
                        if (reply.output.payload.size) {
                            reply.type = com_barobo_rpc_Reply_Type_OUTPUT;
                            reply.has_output = true;
                            reply.output.type = isExceptional ? com_barobo_rpc_Reply_Output_Type_ERROR : com_barobo_rpc_Reply_Output_Type_OUT;
                        }
                        else {
                            reply.type = com_barobo_rpc_Reply_Type_ERROR;
                            reply.has_error = true;
                        }
                    }
                    break;
                case com_barobo_rpc_Request_Type_SUBSCRIBE:
                    reply.type = com_barobo_rpc_Reply_Type_ERROR;
                    reply.has_error = true;
                    reply.error.value = decltype(reply.error.value)(mSubscriptions.activate(request.subscribe.id));
                    break;
                case com_barobo_rpc_Request_Type_UNSUBSCRIBE:
                    reply.type = com_barobo_rpc_Reply_Type_ERROR;
                    reply.has_error = true;
                    reply.error.value = decltype(reply.error.value)(mSubscriptions.deactivate(request.subscribe.id));
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
                    reply.type = com_barobo_rpc_Reply_Type_ERROR;
                    reply.has_error = true;
                    reply.error.value = com_barobo_rpc_Error_ILLEGAL_OPERATION;
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
