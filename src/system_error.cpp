#include "rpc/system_error.hpp"

namespace rpc {

// Local statuses

const char* ErrorCategory::name () const noexcept {
    return "rpc";
}

std::string ErrorCategory::message (int ev) const noexcept {
    return statusToString(static_cast<Status>(ev));
}

const boost::system::error_category& errorCategory () {
    static ErrorCategory instance;
    return instance;
}

boost::system::error_code make_error_code (Status status) {
    return boost::system::error_code(static_cast<int>(status),
        errorCategory());
}

boost::system::error_condition make_error_condition (Status status) {
    return boost::system::error_condition(static_cast<int>(status),
        errorCategory());
}

// Remote statuses

const char* RemoteErrorCategory::name () const noexcept {
    return "rpc-remote";
}

std::string RemoteErrorCategory::message (int ev) const noexcept {
    return statusToString(static_cast<RemoteStatus>(ev));
}

const boost::system::error_category& remoteErrorCategory () {
    static RemoteErrorCategory instance;
    return instance;
}

boost::system::error_code make_error_code (RemoteStatus status) {
    return boost::system::error_code(static_cast<int>(status),
        remoteErrorCategory());
}

boost::system::error_condition make_error_condition (RemoteStatus status) {
    return boost::system::error_condition(static_cast<int>(status),
        remoteErrorCategory());
}


} // namespace rpc