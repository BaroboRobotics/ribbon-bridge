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
// first erroneous error_code received, or success if no errors were reported.
template <class Handler>
class WaitMultipleCompleter {
    struct Impl {
        Impl (boost::asio::io_service& ios, Handler handler)
            : mIos(ios)
            , mHandler(handler)
        {}

        void operator() (boost::system::error_code ec) {
            if (ec) {
                if (mErrorCode) {
                    BOOST_LOG(mLog) << "multiple errors reported to a WaitMultipleCompleter, discarding "
                                    << '"' << ec.message() << '"';
                }
                else {
                    mErrorCode = ec;
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