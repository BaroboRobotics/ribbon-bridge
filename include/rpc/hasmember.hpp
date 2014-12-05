#ifndef RPC_HASMEMBER_HPP
#define RPC_HASMEMBER_HPP

/* For an argument member, define a metafunction with value true if a class T
 * has a member named member.
 * http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 */
#define RPC_DEFINE_TRAIT_HAS_MEMBER(member) \
template <class T> \
struct HasMember##member { \
    struct Yes { char x[1]; }; \
    struct No { char x[10]; }; \
\
    struct Fallback { \
        int member; \
    }; \
\
    struct Derived : T, Fallback { }; \
\
    template <class C> \
    static No check (decltype(&C::member)); \
    template <class> \
    static Yes check (...); \
\
    constexpr static const bool value = sizeof(check<Derived>(0)) == sizeof(Yes); \
};

/* For an argument memfun, define a metafunction with value true if a class T
 * has a member function overload named memfun with function signature
 * R(Args...). */
#define RPC_DEFINE_TRAIT_HAS_MEMBER_FUNCTION_OVERLOAD(memfun) \
template <class T, class F> \
struct HasMemberFunctionOverload##memfun; \
template <class T, class R, class... Args> \
struct HasMemberFunctionOverload##memfun<T, R(Args...)> { \
    struct Yes { char x[1]; }; \
    struct No { char x[10]; }; \
\
    template <class U, R (U::*)(Args...)> \
    struct Check; \
\
    template <class U> \
    static Yes check (Check<U, &U::memfun>*); \
\
    template <class U> \
    static No check (...); \
\
    constexpr static const bool value = sizeof(check<T>(0)) == sizeof(Yes); \
};

#endif
