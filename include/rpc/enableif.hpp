#ifndef RPC_ENABLEIF_HPP
#define RPC_ENABLEIF_HPP

namespace rpc {

template <bool Cond, class T = void>
struct EnableIf { };

template <class T>
struct EnableIf<true, T> { using type = T; };

} // namespace rpc

/* For use as the last, dummy run-time parameter of a function template. */
#define ONLY_IF(x) typename ::rpc::EnableIf<x, void>::type* = 0

// not sure where else to put this stuff
template <bool Cond, class Then, class Else>
struct If { using type = Else; };

template <class Then, class Else>
struct If<true, Then, Else> { using type = Then; };

#endif
