#ifndef RPC_PROXY_HPP
#define RPC_PROXY_HPP

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
    Future<Attribute> get (Attribute, ONLY_IF(IsAttribute<Attribute>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        auto status = makeGet(
                buffer.bytes, buffer.size,
                requestId,
                componentId(Attribute()));
        if (hasError(status)) {
            return static_cast<T*>(this)->template finalize<Attribute>(requestId, status);
        }
        return static_cast<T*>(this)->template finalize<Attribute>(requestId, buffer);
    }

    template <class Attribute>
    Future<void> set (Attribute args, ONLY_IF(IsSettableAttribute<Attribute>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        auto status = makeSet(
                buffer.bytes, buffer.size,
                requestId,
                componentId(Attribute()),
                pbFields(args),
                &args);
        if (hasError(status)) {
            return static_cast<T*>(this)->template finalize<void>(requestId, status);
        }
        return static_cast<T*>(this)->template finalize<void>(requestId, buffer);
    }

    template <class C>
    void broadcast (C args, ONLY_IF(IsSubscribableAttribute<C>::value || IsBroadcast<C>::value)) {
        static_cast<T*>(this)->onBroadcast(args);
    }

    template <class MethodIn>
    Future<typename ResultOf<MethodIn>::type> fire (MethodIn args, ONLY_IF(IsMethod<MethodIn>::value)) {
        using Result = typename ResultOf<MethodIn>::type;
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        auto status = makeFire(
                buffer.bytes, buffer.size,
                requestId,
                componentId(MethodIn()),
                pbFields(args),
                &args);
        if (hasError(status)) {
            return static_cast<T*>(this)->template finalize<Result>(requestId, status);
        }
        return static_cast<T*>(this)->template finalize<Result>(requestId, buffer);
    }

    template <class C>
    Future<void> subscribe (C, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        auto status = makeSubscribe(
                    buffer.bytes, buffer.size,
                    requestId,
                    componentId(C()));
        if (hasError(status)) {
            return static_cast<T*>(this)->template finalize<void>(requestId, status);
        }
        return static_cast<T*>(this)->template finalize<void>(requestId, buffer);
    }

    template <class C>
    Future<void> unsubscribe (C, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        auto status = makeUnsubscribe(
                    buffer.bytes, buffer.size,
                    requestId,
                    componentId(C()));
        if (hasError(status)) {
            return static_cast<T*>(this)->template finalize<void>(requestId, status);
        }
        return static_cast<T*>(this)->template finalize<void>(requestId, buffer);
    }

    Status deliver (BufferType buffer) {
        barobo_rpc_Reply reply;

        auto err = decode(reply, buffer.bytes, buffer.size);
        if (hasError(err)) {
            return err;
        }

        switch (reply.type) {
            ComponentResultUnion<Interface> argument;

            case barobo_rpc_Reply_Type_STATUS:
                if (!reply.has_status) {
                    return Status::INCONSISTENT_REPLY;
                }
                return static_cast<T*>(this)->template fulfill(reply.inReplyTo, static_cast<Status>(reply.status.value));
            case barobo_rpc_Reply_Type_RESULT:
                if (!reply.has_result) {
                    return Status::INCONSISTENT_REPLY;
                }
                err = decodeResultPayload(argument, reply.result.id, reply.result.payload);
                if (!hasError(err)) {
                    err = invokeFulfill(*this, argument, reply.result.id, reply.inReplyTo);
                }
                return err;
            case barobo_rpc_Reply_Type_VERSION:
                if (!reply.has_version) {
                    return Status::INCONSISTENT_REPLY;
                }
                return checkRpcVersion(reply.version.rpc) &&
                    checkInterfaceVersion<Interface>(reply.version.interface) ?
                        Status::OK :
                        Status::VERSION_MISMATCH;
            case barobo_rpc_Reply_Type_BROADCAST:
                if (!reply.has_broadcast) {
                    return Status::INCONSISTENT_REPLY;
                }
                err = decodeBroadcastPayload(argument, reply.broadcast.id, reply.broadcast.payload);
                if (!hasError(err)) {
                    err = invokeBroadcast(*this, argument, reply.broadcast.id);
                }
                return err;
            default:
                return Status::INCONSISTENT_REPLY;
        }

        return Status::OK;
    }

    template <class C>
    Status fulfill (uint32_t requestId, C& result) {
        return static_cast<T*>(this)->fulfill(requestId, result);
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
