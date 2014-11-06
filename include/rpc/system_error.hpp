#ifndef RPC_SYSTEM_ERROR_HPP
#define RPC_SYSTEM_ERROR_HPP

#include "rpc/config.hpp"

#ifndef HAVE_EXCEPTIONS
#error rpc/error.hpp included with exceptions disabled
#endif

#include "rpc/status.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <stdexcept>
#include <string>

namespace rpc {

using Error = boost::system::system_error;

class ErrorCategory : public boost::system::error_category {
public:
    virtual const char* name () const noexcept override;
    virtual std::string message (int ev) const noexcept override;
};

const boost::system::error_category& errorCategory ();
boost::system::error_code make_error_code (Status status);
boost::system::error_condition make_error_condition (Status status);

class RemoteErrorCategory : public boost::system::error_category {
public:
    virtual const char* name () const noexcept override;
    virtual std::string message (int ev) const noexcept override;
};

const boost::system::error_category& remoteErrorCategory ();
boost::system::error_code make_error_code (RemoteStatus status);
boost::system::error_condition make_error_condition (RemoteStatus status);

} // namespace rpc

namespace boost {
namespace system {

template <>
struct is_error_code_enum< ::rpc::Status> : public std::true_type { };

template <>
struct is_error_code_enum< ::rpc::RemoteStatus> : public std::true_type { };

} // namespace system
} // namespace boost

#endif
