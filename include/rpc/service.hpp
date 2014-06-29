#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

#include "rpc/enableif.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/buffer.hpp"
#include "rpc/potqueue.hpp"

namespace rpc {

template <template <class> class Interface>
void decodePayload (ComponentUnion<Interface>& args, com_barobo_rpc_ToObject& toObject);

bool makeBroadcast (uint8_t* buffer, size_t size, uint32_t componentId, const pb_field_t* fields, void* args);

template <class T, template <class> class Interface, size_t QueueSize = 2, size_t BufferSize = 256>
class Service : public Interface<Service<T, Interface>> {
    /* TODO: static_assert that T implements Is.... */
public:
    using BufferType = Buffer<BufferSize>;

    template <class Attribute>
    void on_ (Attribute& args, rpc::Notify, ONLY_IF(IsAttribute<Attribute>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        if (!makeBroadcast(
                buffer.bytes, buffer.size,
                componentId(args),
                pbFields(args),
                &args)) {
            printf("attribute update encoding failed\n");
            return;
        }
        mOutputQueue.push(buffer);
    }

    template <class Broadcast>
    void on_ (Broadcast& args, ONLY_IF(IsBroadcast<Broadcast>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        if (!makeBroadcast(
                buffer.bytes, buffer.size,
                componentId(args),
                pbFields(args),
                &args)) {
            printf("broadcast encoding failed\n");
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
    PotQueue<BufferType, QueueSize> mOutputQueue;
};

} // namespace rpc

#endif
