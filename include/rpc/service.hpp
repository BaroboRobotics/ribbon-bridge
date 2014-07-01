#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

#include "rpc/enableif.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/buffer.hpp"
#include "rpc/message.hpp"
#include "rpc/printmessage.hpp"

namespace rpc {

template <template <class> class Interface>
void decodePayload (ComponentUnion<Interface>& args, uint32_t componentId, com_barobo_rpc_Request_Component_Invocation& invocation);

bool makeBroadcast (uint8_t* buffer, size_t size, uint32_t componentId, const pb_field_t* fields, void* args);

template <template <class> class Interface>
struct Invoker;

template <class T, template <class> class Interface>
void invoke (T& service,
        ComponentUnion<Interface>& argument,
        uint32_t componentId,
        com_barobo_rpc_Request_Component_Invocation& invocation) {
    Invoker<Interface>::invoke(service, argument, componentId, invocation);
}

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
    void on (Attribute& args, rpc::Get, ONLY_IF(IsAttribute<Attribute>)) {
        static_cast<T*>(this)->on(args, rpc::Get());
    }

    template <class Attribute>
    void on (Attribute& args, rpc::Set, ONLY_IF(IsAttribute<Attribute>)) {
        static_cast<T*>(this)->on(args, rpc::Set());
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

    template <class Method>
    void on (Method& args, ONLY_IF(IsMethod<Method>)) {
        static_cast<T*>(this)->on(args);
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

    void deliver (BufferType& buffer) {
        rpc::Request request;
        rpc::decode(request, buffer.bytes, buffer.size);

        printf("Received the following message:\n");
        rpc::printRequest(request);

        if (request.has_component) {
            if (request.component.has_invocation) {
                rpc::ComponentUnion<Interface> argument;
                rpc::decodePayload(argument, request.component.id, request.component.invocation);
                rpc::invoke(*this, argument, request.component.id, request.component.invocation);
            }
            else if (request.component.has_subscription) {
            }
        }
        else if (request.has_status) {
        }
    }
};

} // namespace rpc

#endif
