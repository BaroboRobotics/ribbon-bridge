#ifndef RPC_IMPLEMENTS_HPP
#define RPC_IMPLEMENTS_HPP

#include "rpc/componenttraits.hpp"
#include <utility>

namespace rpc {

struct DummyInterface;

template <>
struct MethodIn<DummyInterface> {
    struct nullaryNoResult { };
};

template <>
struct MethodResult<DummyInterface> {
    struct nullaryNoResult { };
};

struct DummyImpl {
    //MethodResult<DummyInterface>::nullaryNoResult onFire (MethodIn<DummyInterface>::nullaryNoResult);
    int onFire (int);
};

// GENERATED
template <class T>
struct ImplementsOnFireOverloads<T, DummyInterface> {
    static_assert(hasOnFireOverload
            < T
            , MethodResult<DummyInterface>::nullaryNoResult
            , MethodIn<DummyInterface>::nullaryNoResult>::value,
            "does not implement onFire(nullaryNoResult)");
};


static_assert(ImplementsOnFire<DummyImpl, DummyInterface>::value, "does not implement the interface");

} // namespace rpc

#endif
