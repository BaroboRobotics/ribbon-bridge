#ifndef RPC_VERSION_HPP
#define RPC_VERSION_HPP

#include "rpc/config.hpp"

#include "rpc.pb.h"

#ifdef HAVE_STDLIB
#include <iostream>
#endif

namespace rpc {

class VersionTriplet {
public:
	constexpr VersionTriplet (uint32_t major, uint32_t minor, uint32_t patch)
			: mTriplet(barobo_rpc_VersionTriplet{major, minor, patch}) { }

	constexpr VersionTriplet (barobo_rpc_VersionTriplet triplet)
			: mTriplet(triplet) { }

	constexpr operator barobo_rpc_VersionTriplet () const {
		return mTriplet;
	}

	uint32_t major () const { return mTriplet.major; }
	uint32_t minor () const { return mTriplet.minor; }
	uint32_t patch () const { return mTriplet.patch; }

private:
	barobo_rpc_VersionTriplet mTriplet;
};

#ifdef HAVE_STDLIB
inline std::ostream& operator<< (std::ostream& os, const VersionTriplet& triplet) {
	return os << triplet.major() << '.' << triplet.minor() << '.' << triplet.patch();
}

inline std::string to_string (const VersionTriplet& triplet) {
	using std::to_string;
	return to_string(triplet.major()) + '.' +
		   to_string(triplet.minor()) + '.' +
		   to_string(triplet.patch());
}
#endif

inline bool operator== (const VersionTriplet& lhs, const VersionTriplet& rhs) {
    return lhs.major() == rhs.major() &&
           lhs.minor() == rhs.minor() &&
           lhs.patch() == rhs.patch();
}

inline bool operator!= (const VersionTriplet& lhs, const VersionTriplet& rhs) {
	return !(lhs == rhs);
}

template <class Interface = void>
struct Version;

// Define the RPC library version.
template <>
struct Version<void> {
	constexpr static const uint32_t major = RPC_VERSION_MAJOR;
	constexpr static const uint32_t minor = RPC_VERSION_MINOR;
	constexpr static const uint32_t patch = RPC_VERSION_PATCH;
	constexpr static VersionTriplet triplet () { return { major, minor, patch }; }
};

class ServiceInfo {
public:
	ServiceInfo () = default;

	ServiceInfo (barobo_rpc_Reply_ServiceInfo info)
			: mRpcVersion(info.rpcVersion)
			, mInterfaceVersion(info.interfaceVersion) { }

	template <class Interface>
	static ServiceInfo create () {
		barobo_rpc_Reply_ServiceInfo info;
		info.rpcVersion = Version<>::triplet();
		info.interfaceVersion = Version<Interface>::triplet();
		return info;
	}

	operator barobo_rpc_Reply_ServiceInfo () const {
		barobo_rpc_Reply_ServiceInfo info;
        info.rpcVersion = rpcVersion();
        info.interfaceVersion = interfaceVersion();
        return info;
	}

	VersionTriplet rpcVersion () const { return mRpcVersion; }
	VersionTriplet interfaceVersion () const { return mInterfaceVersion; }

private:
	VersionTriplet mRpcVersion = { 0, 0, 0 };
	VersionTriplet mInterfaceVersion = { 0, 0, 0 };
};

} // namespace rpc

#endif