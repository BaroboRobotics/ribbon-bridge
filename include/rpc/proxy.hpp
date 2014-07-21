#ifndef RPC_PROXY_HPP
#define RPC_PROXY_HPP

#include "rpc/buffer.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/message.hpp"
#include "rpc/enableif.hpp"
#include "rpc/checkversion.hpp"
#include "rpc.pb.h"

namespace rpc {

template <class T, class Interface, template <class> class RequestManager>
class Proxy {
public:
    using BufferType = Buffer<256>;

    Proxy () { (void)AssertProxyImplementsInterface<T, Interface>(); }

    template <class U>
    using Future = typename RequestManager<Interface>::template Future<U>;

    template <class Attribute>
    Future<Attribute> get (Attribute, ONLY_IF(IsAttribute<Attribute>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.nextRequestId();
        auto status = makeGet(
                buffer.bytes, buffer.size,
                requestId,
                componentId(Attribute()));
        if (hasError(status)) {
            return mRequestManager.template finalize<Attribute>(requestId, status);
        }
        auto future = mRequestManager.template finalize<Attribute>(requestId);
        static_cast<T*>(this)->post(buffer);
        return future;
    }

    template <class Attribute>
    Future<void> set (Attribute args, ONLY_IF(IsSettableAttribute<Attribute>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.nextRequestId();
        auto status = makeSet(
                buffer.bytes, buffer.size,
                requestId,
                componentId(Attribute()),
                pbFields(args),
                &args);
        if (hasError(status)) {
            return mRequestManager.template finalize<void>(requestId, status);
        }
        auto future = mRequestManager.template finalize<void>(requestId);
        static_cast<T*>(this)->post(buffer);
        return future;
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
        auto requestId = mRequestManager.template nextRequestId();
        auto status = makeFire(
                buffer.bytes, buffer.size,
                requestId,
                componentId(MethodIn()),
                pbFields(args),
                &args);
        if (hasError(status)) {
            return mRequestManager.template finalize<Result>(requestId, status);
        }

        /* We must call finalize() before post(). If the opposite order were
         * used, we could potentially end up with an encoded message on the wire
         * with no promise or future generated for it yet. */
        auto future = mRequestManager.template finalize<Result>(requestId);
        static_cast<T*>(this)->post(buffer);
        return future;
    }

    template <class C>
    Future<void> subscribe (C, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.nextRequestId();
        auto status = makeSubscribe(
                    buffer.bytes, buffer.size,
                    requestId,
                    componentId(C()));
        if (hasError(status)) {
            return mRequestManager.template finalize<void>(requestId, status);
        }
        auto future = mRequestManager.template finalize<void>(requestId);
        static_cast<T*>(this)->post(buffer);
        return future;
    }

    template <class C>
    Future<void> unsubscribe (C, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.template nextRequestId();
        auto status = makeUnsubscribe(
                    buffer.bytes, buffer.size,
                    requestId,
                    componentId(C()));
        if (hasError(status)) {
            return mRequestManager.template finalize<void>(requestId, status);
        }
        auto future = mRequestManager.template finalize<void>(requestId);
        static_cast<T*>(this)->post(buffer);
        return future;
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
                return mRequestManager.fulfill(reply.inReplyTo, static_cast<Status>(reply.status.value));
            case barobo_rpc_Reply_Type_RESULT:
                if (!reply.has_result) {
                    return Status::INCONSISTENT_REPLY;
                }
                err = decodeResultPayload(argument, reply.result.id, reply.result.payload);
                if (!hasError(err)) {
                    err = invokeFulfill(mRequestManager, argument, reply.result.id, reply.inReplyTo);
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

private:
    RequestManager<Interface> mRequestManager;
};

} // namespace rpc

#endif
