#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

#include "rpc/enableif.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/buffer.hpp"

namespace rpc {

template <template <class> class Interface>
void decodePayload (ComponentUnion<Interface>& args, uint32_t componentId, com_barobo_rpc_Request_Component_Invocation& invocation);

bool makeBroadcast (uint8_t* buffer, size_t size, uint32_t componentId, const pb_field_t* fields, void* args);

template <class T, template <class> class Interface>
class Service {
    /* TODO: static_assert that Service is standard layout and mInterface is
     * the initial data member. */
    Interface<Service> mInterface;

    /* TODO: static_assert that T implements Is.... */
public:
    using BufferType = Buffer<256>;

    Interface<Service>* operator-> () {
        return &mInterface;
    }

    const Interface<Service>* operator-> () const {
        return &mInterface;
    }

    template <class Attribute>
    void on (Attribute& args, rpc::Notify, ONLY_IF(IsAttribute<Attribute>)) {
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
        static_cast<T*>(this)->post(buffer);
    }

    template <class Broadcast>
    void on (Broadcast& args, ONLY_IF(IsBroadcast<Broadcast>)) {
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
        static_cast<T*>(this)->post(buffer);
    }
};

} // namespace rpc

#endif
