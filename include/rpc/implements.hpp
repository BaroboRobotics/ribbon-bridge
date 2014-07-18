#ifndef RPC_IMPLEMENTS_HPP
#define RPC_IMPLEMENTS_HPP

#include "rpc/componenttraits.hpp"

namespace rpc {

// value will be true if T implements an overload of onFire accepting an
// argument of type A and returning a value of type R.
template <class T, class R, class A>
struct HasOnFireOverload {
    struct Yes { char x[1]; };
    struct No { char x[10]; };

    template <class U, R (U::*)(A)>
    struct Check;

    template <class U>
    static Yes check (Check<U, &U::onFire>*);

    template <class U>
    static No check (...);

    constexpr static const bool value = sizeof(check<T>(0)) == sizeof(Yes);
};

// specialized by generated code
template <class T, class Interface>
struct ImplementsOnFireOverloads;

/* Detect if the class T implements an onFire method, then test each and every
 * required overload thereof. If no onFire method is found, or a required
 * overload is missing, a static assertion will stop compilation.
 *
 * http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 *
 * Use like so: static_assert(ServiceImplements<Impl, Interface>::value, "does
 * not implement the interface");
 */
template <class T, class Interface>
struct ServiceImplements {
    struct Yes { char x[1]; };
    struct No { char x[10]; };

    struct Fallback {
        int onGet;
        int onSet;
        int onFire;
    };

    struct Derived : T, Fallback { };

    template <class C>
    static No checkOnGet (decltype(&C::onGet));
    template <class>
    static Yes checkOnGet (...);

    template <class C>
    static No checkOnSet (decltype(&C::onSet));
    template <class>
    static Yes checkOnSet (...);

    template <class C>
    static No checkOnFire (decltype(&C::onFire));
    template <class>
    static Yes checkOnFire (...);

    constexpr static const bool hasOnGet = sizeof(checkOnGet<Derived>(0)) == sizeof(Yes);
    constexpr static const bool hasOnSet = sizeof(checkOnSet<Derived>(0)) == sizeof(Yes);
    constexpr static const bool hasOnFire = sizeof(checkOnFire<Derived>(0)) == sizeof(Yes);


    static_assert(hasOnGet, "does not implement onGet");
    static_assert(hasOnSet, "does not implement onSet");
    static_assert(hasOnFire, "does not implement onFire");

    static typename If<, ServiceImplementsOverloads<T, Interface>, void*>::type overloadTester;
};

#endif
