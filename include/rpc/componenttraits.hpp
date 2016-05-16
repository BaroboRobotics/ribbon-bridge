#ifndef RPC_COMPONENTTRAITS_HPP
#define RPC_COMPONENTTRAITS_HPP

#include <rpc/config.hpp>
#include <rpc/stdlibheaders.hpp>
#include <rpc/status.hpp>
#include <rpc/hasmember.hpp>

namespace rpc {

// Access the method component messages of an interface by name.
template <class Interface>
struct MethodIn;

template <class Interface>
struct MethodResult;

// Access the broadcast component messages of an interface by name.
template <class Interface>
struct Broadcast;

// Metafunction to identify whether a type is a method component message.
template <class Method>
struct IsMethod { static const bool value = false; };

// Metafunction to identify whether a type is a broadcast component message.
template <class Broadcast>
struct IsBroadcast { static const bool value = false; };

// Union containing a data member for every component message of an interface,
// and an invoke member, such that:
//   MethodInUnion<Interface>().invoke(server, id, inPayload, outPayload, status);
// executes the method specified by the given component id, using the arguments
// encoded in the barobo_rpc_Request_Fire_payload_t parameter, inPayload, and
// encodes the result in the barobo_rpc_Reply_Result_payload_t parameter,
// outPayload.
template <class Interface>
union MethodInUnion;

template <class Interface>
union BroadcastUnion;

// Access the component ID of an interface by type.
template <class Payload>
#if HAVE_CONSTEXPR_FUNCTION_TEMPLATES
constexpr
#else
static inline
#endif // HAVE_CONSTEXPR_FUNCTION_TEMPLATES
uint32_t componentId (Payload);

template <class T>
struct ResultOf;

template <class T>
struct ResultOf<T&> : ResultOf<T> { };

RPC_DEFINE_TRAIT_HAS_MEMBER_FUNCTION_OVERLOAD(onFire)
RPC_DEFINE_TRAIT_HAS_MEMBER_FUNCTION_OVERLOAD(onBroadcast)

template <class T, class Interface>
struct AssertServerImplementsInterface;

template <class T, class Interface>
struct AssertClientImplementsInterface;

} // namespace rpc

#endif
