#ifndef RPC_EXCEPTION_HPP
#define RPC_EXCEPTION_HPP

#ifndef HAVE_EXCEPTIONS
#error exception.hpp included with exceptions disabled
#endif

#include <exception>
#include <string>

namespace rpc {

struct RpcError : std::runtime_error {
    // when we can use g++ 4.8+, uncomment and destroy that stupid ctor
    //using std::runtime_error::runtime_error;
    explicit RpcError (const std::string& what)
        : std::runtime_error(what) { }
};

} // namespace rpc

#endif
