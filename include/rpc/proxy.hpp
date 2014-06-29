#ifndef RPC_PROXY_HPP
#define RPC_PROXY_HPP

#include "rpc/stdlibheaders.hpp"
#include "rpc/buffer.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/message.hpp"
#include "rpc/potqueue.hpp"
#include "rpc/enableif.hpp"
#include "rpc.pb.h"

namespace rpc {

bool makeMessage (uint8_t* buffer, size_t& size, com_barobo_rpc_ToObject_Type, uint32_t messageId, uint32_t componentId, const pb_field_t* fields, void* args);

template <template <class> class Interface, size_t QueueSize = 2, size_t BufferSize = 256>
class Proxy : public Interface<Proxy<Interface>> {
public:
    using BufferType = Buffer<BufferSize>;

    template <class Method>
    void on_ (Method& args, ONLY_IF(IsMethod<Method>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        if (!makeMessage(
                buffer.bytes, buffer.size,
                com_barobo_rpc_ToObject_Type_FIRE,
                nextMessageId(),
                componentId(args),
                pbFields(args.in),
                &args.in)) {
            printf("shit\n");
            return;
        }
        mOutputQueue.push(buffer);
    }

    template <class Attribute>
    void on_ (Attribute& args, rpc::Get, ONLY_IF(IsAttribute<Attribute>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        if (!makeMessage(
                buffer.bytes, buffer.size,
                com_barobo_rpc_ToObject_Type_GET,
                nextMessageId(),
                componentId(args),
                pbFields(args),
                &args)) {
            printf("shit\n");
            return;
        }
        mOutputQueue.push(buffer);
    }

    template <class Attribute>
    void on_ (Attribute& args, rpc::Set, ONLY_IF(IsAttribute<Attribute>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        if (!makeMessage(
                buffer.bytes, buffer.size,
                com_barobo_rpc_ToObject_Type_SET,
                nextMessageId(),
                componentId(args),
                nullptr,
                nullptr)) {
            printf("shit\n");
            return;
        }
        mOutputQueue.push(buffer);
    }

    /* obviously not thread-safe */
    bool tryPop_ (BufferType& buffer) {
        if (!mOutputQueue.empty()) {
            buffer = mOutputQueue.front();
            mOutputQueue.pop();
            return true;
        }
        return false;
    }

private:
    /* also not thread-safe */
    uint32_t nextMessageId () {
        return mNextMessageId++;
    }

    uint32_t mNextMessageId = 0;
    PotQueue<BufferType, QueueSize> mOutputQueue;
};

} // namespace rpc

#endif
