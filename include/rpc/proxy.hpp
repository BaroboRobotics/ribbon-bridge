#ifndef RPC_PROXY_HPP
#define RPC_PROXY_HPP

#include "rpc/stdlibheaders.hpp"
#include "rpc/buffer.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/message.hpp"
#include "rpc/enableif.hpp"
#include "rpc/checkversion.hpp"
#include "rpc.pb.h"

namespace rpc {

template <class T, class Interface, template <class> class Future>
class Proxy {
    /* TODO: static_assert that T implements Is.... */
public:
    using BufferType = Buffer<256>;

    template <class Attribute>
    Future<Attribute> get (Attribute args, ONLY_IF(IsAttribute<Attribute>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        if (hasError(makeGet(
                buffer.bytes, buffer.size,
                requestId,
                componentId(args)))) {
            printf("shit\n");
            return static_cast<T*>(this)->template finalize<Attribute>(requestId, componentId(args));
        }
        return static_cast<T*>(this)->template finalize<Attribute>(requestId, componentId(args), buffer);
    }

    template <class Attribute>
    Future<void> set (Attribute args, ONLY_IF(IsAttribute<Attribute>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        if (hasError(makeSet(
                buffer.bytes, buffer.size,
                requestId,
                componentId(args),
                pbFields(args),
                &args))) {
            printf("shit\n");
            return static_cast<T*>(this)->template finalize<Attribute>(requestId, componentId(args));
        }
        return static_cast<T*>(this)->template finalize<Attribute>(requestId, componentId(args), buffer);
    }

    template <class C>
    void broadcast (C args, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        static_cast<T*>(this)->broadcast(args);
    }

    template <class MethodIn>
    Future<typename ResultOf<MethodIn>::type> on (MethodIn args, ONLY_IF(IsMethod<MethodIn>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        auto error = makeFire(
                buffer.bytes, buffer.size,
                requestId,
                componentId(args),
                pbFields(args),
                &args);
        if (hasError(error)) {
            printf("shit\n");
            return static_cast<T*>(this)->template finalize<typename ResultOf<MethodIn>::type>(requestId, componentId(args), error);
        }
        return static_cast<T*>(this)->template finalize<typename ResultOf<MethodIn>::type>(requestId, componentId(args), buffer);
    }

    template <class C>
    Future<void> subscribe (C, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId;
        auto error = makeSubscribe(
                    buffer.bytes, buffer.size,
                    requestId,
                    componentId(C()));
        if (hasError(error)) {
            printf("shit\n");
            return static_cast<T*>(this)->template finalize<C>(requestId, componentId(C()), error);
        }
        return static_cast<T*>(this)->template finalize<C>(requestId, componentId(C()), buffer);
    }

    template <class C>
    Future<void> unsubscribe (C, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId;
        auto error = makeUnsubscribe(
                    buffer.bytes, buffer.size,
                    requestId,
                    componentId(C()));
        if (hasError(error)) {
            printf("shit\n");
            return static_cast<T*>(this)->template finalize<C>(requestId, componentId(C()), error);
        }
        return static_cast<T*>(this)->template finalize<C>(requestId, componentId(C()), buffer);
    }

    Error deliver (BufferType buffer) {
        com_barobo_rpc_Reply reply;

        auto err = decode(reply, buffer.bytes, buffer.size);
        if (hasError(err)) {
            return err;
        }

        printf("Received the following reply:\n");
        //printReply(reply);

        switch (reply.type) {
            ComponentOutUnion<Interface> argument;

            case com_barobo_rpc_Reply_Type_ERROR:
                if (!reply.has_error) {
                    return Error::INCONSISTENT_REPLY;
                }
                return static_cast<T*>(this)->fulfillWithError(reply.inReplyTo, Error(reply.error.value));
            case com_barobo_rpc_Reply_Type_OUTPUT:
                if (!reply.has_output) {
                    return Error::INCONSISTENT_REPLY;
                }
                err = decodeOutputPayload(argument, reply.output.id, reply.output.payload);
                if (!hasError(err)) {
                    err = invokeFulfillWithOutput(*this, argument, reply.output.id, reply.inReplyTo);
                }
                return err;
            case com_barobo_rpc_Reply_Type_VERSION:
                if (!reply.has_version) {
                    return Error::INCONSISTENT_REPLY;
                }
                return checkRpcVersion(reply.version.rpc) &&
                    checkInterfaceVersion<Interface>(reply.version.interface) ?
                        Error::NO_ERROR :
                        Error::VERSION_MISMATCH;
            case com_barobo_rpc_Reply_Type_BROADCAST:
                if (reply.has_broadcast) {
                    return Error::INCONSISTENT_REPLY;
                }
                err = decodeBroadcastPayload(argument, reply.broadcast.id, reply.broadcast.payload);
                if (!hasError(err)) {
                    err = invokeBroadcast(*this, argument, reply.broadcast.id);
                }
                return err;
            default:
                return Error::INCONSISTENT_REPLY;
        }

        return Error::NO_ERROR;
    }

    template <class Out>
    Error fulfillWithOutput (uint32_t requestId, Out& out) {
        return static_cast<T*>(this)->fulfillWithOutput(requestId, out);
    }

private:
    /* not thread safe */
    uint32_t nextRequestId () {
        return mNextRequestId++;
    }

    uint32_t mNextRequestId = 0;
};

} // namespace rpc

#endif
