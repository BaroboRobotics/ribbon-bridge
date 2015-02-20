#ifndef RPC_ASIO_WAITMULTIPLECOMPLETER_HPP
#define RPC_ASIO_WAITMULTIPLECOMPLETER_HPP

#include <boost/asio/io_service.hpp>

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <memory>
#include <functional>
#include <exception>

namespace rpc {
namespace asio {

// Create an object of this type, then copy it as much as you want, passing it
// as a handler to multiple asynchronous operations producing an error_code.
// When the last copy is destroyed, the original handler is posted with the
// first non-operation_aborted error_code received, or operation_aborted if
// only operation_aborted and success are received, or success if only success
// error_codes were received.
template <class Handler>
class WaitMultipleCompleter {
    struct Impl {
        Impl (boost::asio::io_service& ios, Handler handler)
            : mIos(ios)
            , mHandler(handler)
        {}

        // We consider three kinds of errors: success, operation_aborted, and
        // all others. In general, the "all others" category is the most
        // interesting, because it is the only true error: operation_aborted
        // just means that we, the programmer, canceled the operation.
        // WaitMultipleCompleter::operator()'s job is to collect these errors
        // such that mErrorCode contains the first reported non-success, non-
        // operation_aborted error. If more than one such error is reported, we
        // can squawk about it in the log. If no such error is reported,
        // mErrorCode shall contain the logical OR of all reported errors,
        // where operation_aborted is 1, success is 0.
        void operator() (boost::system::error_code ec) {
            if (ec) {
                // If we haven't recorded an error in the "all others" category yet...
                if (!mErrorCode || boost::asio::error::operation_aborted == mErrorCode) {
                    mErrorCode = ec;
                }
                // If we have recorded an error in the "all others" category, but
                // we have to deal with a second one...
                else if (boost::asio::error::operation_aborted != ec) {
                    BOOST_LOG(mLog) << "multiple non-operation_aborted errors reported to a WaitMultipleCompleter, discarding "
                                    << '"' << ec.message() << '"';
                }
            }
        }

        static void deleter (Impl* p) {
            try {
                p->mIos.post(std::bind(p->mHandler, p->mErrorCode));
            }
            catch (std::exception& e) {
                BOOST_LOG(p->mLog) << "Exception posting WaitMultipleCompleter handler: " << e.what();
            }
            delete p;
        }

        mutable boost::log::sources::logger mLog;

        boost::asio::io_service& mIos;
        Handler mHandler;
        boost::system::error_code mErrorCode;
    };

    std::shared_ptr<Impl> mImpl;

public:
    WaitMultipleCompleter (boost::asio::io_service& ios, Handler handler)
        : mImpl(new Impl(ios, handler), &Impl::deleter)
    {}

    void operator() (boost::system::error_code ec) {
        (*mImpl)(ec);
    }
};

} // namespace asio
} // namespace rpc

#endif