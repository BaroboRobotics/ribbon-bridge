#ifndef RPC_SERVICE_HPP
#define RPC_SERVICE_HPP

namespace rpc {

template <template <class> class Interface>
struct Method;

class Service {
public:
    template <class Object>
    void registerObject (Object&& object) {
        (void)object;
    }

private:
};

template <class T, class I = typename T::Interface, class... Is>
class Object : public I, public Is... {
    /* TODO: static_assert that T implements I, Is.... */
public:
    Object () : I(mInstance), Is(mInstance)..., mInstance(*this) { }

private:
    T mInstance;
};

} // namespace rpc

#endif
