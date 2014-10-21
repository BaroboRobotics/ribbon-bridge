#ifndef RPC_ERROR_HPP
#define RPC_ERROR_HPP

#include "rpc/config.hpp"
#include "rpc/status.hpp"

#ifndef HAVE_EXCEPTIONS
#error rpc/error.hpp included with exceptions disabled
#endif

#include <stdexcept>
#include <string>

namespace rpc {

struct Error : std::runtime_error {
    // when we can use g++ 4.8+, uncomment and destroy that stupid ctor
    //using std::runtime_error::runtime_error;
    explicit Error (const std::string& what)
        : std::runtime_error(what) { }

    explicit Error (Status status)
        : std::runtime_error(statusToString(status)) { }
};

} // namespace rpc

#endif
