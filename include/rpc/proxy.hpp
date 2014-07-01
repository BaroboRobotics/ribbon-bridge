#ifndef RPC_PROXY_HPP
#define RPC_PROXY_HPP

#include "rpc/stdlibheaders.hpp"
#include "rpc/buffer.hpp"
#include "rpc/componenttraits.hpp"
#include "rpc/implementationtraits.hpp"
#include "rpc/message.hpp"
#include "rpc/enableif.hpp"
#include "rpc.pb.h"

namespace rpc {

bool makeRequestComponentInvocation (uint8_t* buffer, size_t& size, uint32_t requestId, uint32_t componentId, const pb_field_t* fields, void* args);

template <class T>
struct ProxyTraits;

template <class T, template <class> class Interface>
class Proxy {
    /* TODO: static_assert that Service is standard layout and mInterface is
     * the initial data member. */
    Interface<Proxy> mInterface;

    /* TODO: static_assert that T implements Is.... */
public:
    using BufferType = Buffer<256>;

    Interface<Proxy>* operator-> () {
        return &mInterface;
    }

    const Interface<Proxy>* operator-> () const {
        return &mInterface;
    }

    template <class C>
    using ReturnType = typename ImplementationTraits<T>::template ReturnType<C>;

    template <class Attribute>
    ReturnType<Attribute> on (Attribute& args, rpc::Get, ONLY_IF(IsAttribute<Attribute>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        if (!makeRequestComponentInvocation(
                buffer.bytes, buffer.size,
                requestId,
                componentId(args),
                pbFields(args),
                &args)) {
            printf("shit\n");
            return static_cast<T*>(this)->template promise<Attribute>();
        }
        static_cast<T*>(this)->post(buffer);
        return static_cast<T*>(this)->template promise<Attribute>(requestId);
    }

    template <class Attribute>
    ReturnType<void> on (Attribute& args, rpc::Set, ONLY_IF(IsAttribute<Attribute>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        if (!makeRequestComponentInvocation(
                buffer.bytes, buffer.size,
                requestId,
                componentId(args),
                nullptr,
                nullptr)) {
            printf("shit\n");
            return static_cast<T*>(this)->template promise<Attribute>();
        }
        static_cast<T*>(this)->post(buffer);
        return static_cast<T*>(this)->template promise<Attribute>(requestId);
    }

    template <class Method>
    ReturnType<Method> on (Method& args, ONLY_IF(IsMethod<Method>)) {
        BufferType buffer;
        buffer.size = sizeof(buffer.bytes);
        auto requestId = nextRequestId();
        if (!makeRequestComponentInvocation(
                buffer.bytes, buffer.size,
                requestId,
                componentId(args),
                pbFields(args.in),
                &args.in)) {
            printf("shit\n");
            return static_cast<T*>(this)->template promise<Method>();
        }
        static_cast<T*>(this)->post(buffer);
        return static_cast<T*>(this)->template promise<Method>(requestId);
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
