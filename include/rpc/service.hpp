#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

#define RPC_MESSAGE_MAX_SIZE 256

namespace rpc {

/* Buffer to hold encoded rpc messages. */
struct Buffer {
    size_t size;
    uint8_t bytes[RPC_MESSAGE_MAX_SIZE];
};

/* Metafunction to access the input, output, and error parameter structures of
 * an interface's methods. Specialized in generated code. */
template <template <class> class Interface>
struct Method;

template <class T, template <class> class... Is>
class Object;

template <class T, template <class> class... Is>
T& getInstance (Object<T, Is...>&);

template <class T, template <class> class... Is>
class Object : public Is<Object<T, Is...>>... {
    /* TODO: static_assert that T implements Is.... */
    friend T& getInstance<> (Object& object);

    T mInstance;
};

template <class T, template <class> class... Is>
T& getInstance (Object<T, Is...>& object) {
    return object.mInstance;
}

} // namespace rpc

#endif
