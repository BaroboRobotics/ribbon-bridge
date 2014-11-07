#ifndef RPC_PROXY_HPP
#define RPC_PROXY_HPP

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include "rpc/buffer.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/message.hpp"
#include "rpc/enableif.hpp"
#include "rpc/version.hpp"
#include "rpc.pb.h"

namespace rpc {

template <class T, class Interface, template <class> class RequestManager>
class Proxy {
public:
    using BufferType = Buffer<256>;

    Proxy () { (void)AssertProxyImplementsInterface<T, Interface>(); }

    template <class U>
    using Future = typename RequestManager<Interface>::template Future<U>;

    template <class C>
    void broadcast (C args, ONLY_IF(IsBroadcast<C>::value)) {
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

        /* We must call finalize() before bufferToService(). If the opposite order were
         * used, we could potentially end up with an encoded message on the wire
         * with no promise or future generated for it yet. */
        auto future = mRequestManager.template finalize<Result>(requestId);
        static_cast<T*>(this)->bufferToService(buffer);
        return future;
    }

    Future<ServiceInfo> connect () {
        BOOST_LOG_NAMED_SCOPE("rpc::Proxy::connect");
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.template nextRequestId();
        auto status = makeConnect(
                    buffer.bytes, buffer.size,
                    requestId);
        if (hasError(status)) {
            return mRequestManager.template finalize<ServiceInfo>(requestId, status);
        }
        auto future = mRequestManager.template finalize<ServiceInfo>(requestId);
        BOOST_LOG(mLog) << "created connection request, sending to service";
        static_cast<T*>(this)->bufferToService(buffer);
        return future;
    }

    Future<void> disconnect () {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.template nextRequestId();
        auto status = makeDisconnect(
                    buffer.bytes, buffer.size,
                    requestId);
        if (hasError(status)) {
            return mRequestManager.template finalize<void>(requestId, status);
        }
        auto future = mRequestManager.template finalize<void>(requestId);
        static_cast<T*>(this)->bufferToService(buffer);
        return future;
    }

#if 0
    // TODO
    Future<void> disconnect () {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = mRequestManager.template nextRequestId();
        auto status = makeDisconnect(
                    buffer.bytes, buffer.size);
        if (hasError(status)) {
            return mRequestManager.template finalize<void>(requestId, status);
        }
        auto future = mRequestManager.template finalize<void>(requestId);
        static_cast<T*>(this)->bufferToService(buffer);
        return future;
    }
#endif

    Status receiveServiceBuffer (BufferType buffer) {
        barobo_rpc_ServerMessage message;

        Status status;
        decode(message, buffer.bytes, buffer.size, status);
        if (hasError(status)) {
            return status;
        }

        switch (message.type) {
            ComponentResultUnion<Interface> argument;

            case barobo_rpc_ServerMessage_Type_REPLY:
                if (!message.has_reply || !message.has_inReplyTo) {
                    return Status::INCONSISTENT_REPLY;
                }
                switch (message.reply.type) {
                    case barobo_rpc_Reply_Type_STATUS:
                        if (!message.reply.has_status) {
                            return Status::INCONSISTENT_REPLY;
                        }
                        return mRequestManager.fulfill(message.inReplyTo, static_cast<Status>(message.reply.status.value));
                    case barobo_rpc_Reply_Type_RESULT:
                        if (!message.reply.has_result) {
                            return Status::INCONSISTENT_REPLY;
                        }
                        status = decodeResultPayload(argument, message.reply.result.id, message.reply.result.payload);
                        if (!hasError(status)) {
                            status = invokeFulfill(mRequestManager, argument, message.reply.result.id, message.inReplyTo);
                        }
                        return status;
                    case barobo_rpc_Reply_Type_SERVICEINFO:
                        if (!message.reply.has_serviceInfo) {
                            return Status::INCONSISTENT_REPLY;
                        }
                        return mRequestManager.fulfill(message.inReplyTo, ServiceInfo(message.reply.serviceInfo));
                    default:
                        return Status::INCONSISTENT_REPLY;
                }
                break;
            case barobo_rpc_ServerMessage_Type_BROADCAST:
                if (!message.has_broadcast) {
                    return Status::INCONSISTENT_REPLY;
                }
                status = decodeBroadcastPayload(argument, message.broadcast.id, message.broadcast.payload);
                if (!hasError(status)) {
                    status = invokeBroadcast(*this, argument, message.broadcast.id);
                }
                return status;
            default:
                return Status::INCONSISTENT_REPLY;
        }

        return Status::OK;
    }

private:
    boost::log::sources::logger_mt mLog;

    RequestManager<Interface> mRequestManager;
};

} // namespace rpc

#endif
