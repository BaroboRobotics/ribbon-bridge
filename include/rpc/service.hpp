#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#include "rpc.pb.h"

namespace rpc {

/* Tag dispatching for setter and getter functions. */
struct Set { };
struct Get { };

/* Metafunction to access the attribute structures of an interface. Specialized
 * in generated code. */
template <template <class> class Interface>
struct Attribute;

/* Metafunction to access the input, output, and error parameter structures of
 * an interface's methods. Specialized in generated code. */
template <template <class> class Interface>
struct Method;

/* Metafunction to access the broadcast structures of an interface. Specialized
 * in generated code. */
template <template <class> class Interface>
struct Broadcast;

template <template <class> class Interface>
union ArgumentUnion;

template <template <class> class Interface>
void decodePayload (ArgumentUnion<Interface>& args, com_barobo_rpc_ToObject& toObject);

template <template <class> class Interface>
struct ComponentId;

template <class T, template <class> class... Is>
class Service : public Is<Service<T, Is...>>... {
    /* TODO: static_assert that T implements Is.... */
public:
    using Subscription = uint32_t;

protected:
    template <class B>
    void broadcast_ (B& args) {
        printf("Broadcasting ...\n");
    }

private:
    template <class B>
    Subscription subscribe_ () {
        printf("Someone wants to subscribe ...\n");
        return 0;
    }


};

} // namespace rpc

#endif
