#ifndef RPC_VERSION_HPP
#define RPC_VERSION_HPP

#include "rpc/config.hpp"

#include "rpc.pb.h"

#ifdef HAVE_STDLIB
#include <iostream>
#include <string>
#endif

namespace rpc {

class VersionTriplet {
public:
	VersionTriplet (uint32_t major, uint32_t minor, uint32_t patch)
			: mTriplet(barobo_rpc_VersionTriplet{major, minor, patch}) { }

	explicit VersionTriplet (barobo_rpc_VersionTriplet triplet)
			: mTriplet(triplet) { }

	operator barobo_rpc_VersionTriplet () const {
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
	static const uint32_t major = RPC_VERSION_MAJOR;
	static const uint32_t minor = RPC_VERSION_MINOR;
	static const uint32_t patch = RPC_VERSION_PATCH;
	static VersionTriplet triplet () { return { major, minor, patch }; }
};

class Versions {
public:
	Versions () = default;

	Versions (barobo_rpc_Versions vers)
			: mRpcVersion(vers.rpc)
			, mInterfaceVersion(vers.interface) { }

	template <class Interface>
	static Versions create () {
		barobo_rpc_Versions vers;
		vers.rpc = Version<>::triplet();
		vers.interface = Version<Interface>::triplet();
		return vers;
	}

	operator barobo_rpc_Versions () const {
		barobo_rpc_Versions vers;
        vers.rpc = rpc();
        vers.interface = interface();
        return vers;
	}

	VersionTriplet rpc () const { return mRpcVersion; }
	VersionTriplet interface () const { return mInterfaceVersion; }

private:
	VersionTriplet mRpcVersion = { 0, 0, 0 };
	VersionTriplet mInterfaceVersion = { 0, 0, 0 };
};

} // namespace rpc

#endif
