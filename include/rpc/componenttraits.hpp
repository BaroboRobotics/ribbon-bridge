#ifndef RPC_COMPONENTTRAITS_HPP
#define RPC_COMPONENTTRAITS_HPP

namespace rpc {

/* Tag dispatching for setter, getter, and notification functions. */
struct Set { };
struct Get { };
struct Notify { };

// Access the attribute component messages of an interface by name.
template <template <class> class Interface>
struct Attribute;

// Access the method component messages of an interface by name.
template <template <class> class Interface>
struct Method;

// Access the broadcast component messages of an interface by name.
template <template <class> class Interface>
struct Broadcast;

// Identify whether a type is an attribute component message.
template <class Attribute>
struct IsAttribute { constexpr static const bool value = false; };

// Metafunction to identify whether a type is a method component message.
template <class Method>
struct IsMethod { constexpr static const bool value = false; };

// Metafunction to identify whether a type is a broadcast component message.
template <class Broadcast>
struct IsBroadcast { constexpr static const bool value = false; };

// Union containing a data member for every component message of an interface.
template <template <class> class Interface>
union ComponentUnion;

// Access the component IDs of an interface by name.
template <template <class> class Interface>
struct ComponentId;

// Access the component ID of an interface by type.
template <class Payload>
constexpr uint32_t componentId (Payload);

} // namespace rpc

#endif
