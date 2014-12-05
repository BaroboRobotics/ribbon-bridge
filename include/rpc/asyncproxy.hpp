#ifndef RPC_ASYNCPROXY_HPP
#define RPC_ASYNCPROXY_HPP

#include "rpc/config.hpp"

#ifndef HAVE_STDLIB
#error "this file requires the standard library"
#endif

#include "rpc/system_error.hpp"
#include "rpc/proxy.hpp"

#include "util/deadlinescheduler.hpp"
#include "util/variant.hpp"

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include <boost/unordered_map.hpp>

#include <future>
#include <tuple>
#include <utility>

#include <cstdio>
#include <cinttypes>

namespace rpc {

template <class Interface>
class AsyncRequestManager {
    template <class... Ts>
    using MakePromiseVariant = util::Variant<std::promise<Ts>...>;

    using PromiseVariant = typename rpc::PromiseVariadic<Interface, MakePromiseVariant>::type;

    struct Throw {
        explicit Throw (std::exception_ptr e) : mException(e) { }

        template <class P>
        void operator() (P& promise) const { promise.set_exception(mException); }

    private:
        std::exception_ptr mException;
    };

public:
    template <class T>
    using Future = std::future<T>;

    // Fulfill a promise with an error from the remote host.
    Status fulfill (uint32_t requestId, Status status) {
        BOOST_LOG_NAMED_SCOPE("rpc::AsyncRequestManager::fulfill(status)");

        std::lock_guard<decltype(mPromisesMutex)> lock { mPromisesMutex };

        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            BOOST_LOG(mLog) << "request " << requestId << " does not exist";
            return Status::UNSOLICITED_REPLY;
        }

        auto promisePtr = util::get<std::promise<void>>(&iter->second);
        if (promisePtr) {
            BOOST_LOG(mLog) << "fulfilling request " << requestId << "with "
                            << statusToString(status);
            if (!hasError(status)) {
                promisePtr->set_value();
            }
            else {
                auto eptr = std::make_exception_ptr(Error(status));
                util::apply(Throw(eptr), iter->second);
            }
        }
        else {
            BOOST_LOG(mLog) << "type mismatch fulfilling request " << requestId
                            << " with " << statusToString(status);
            if (!hasError(status)) {
                // No error reported, but we have a type mismatch.
                printf("type mismatch with requestId %" PRId32 "\n", requestId);
                auto eptr = std::make_exception_ptr(Error(Status::UNRECOGNIZED_RESULT));
                util::apply(Throw(eptr), iter->second);
            }
            else {
                auto eptr = std::make_exception_ptr(Error(status));
                util::apply(Throw(eptr), iter->second);
            }
        }

        mPromises.erase(iter);
        return Status::OK;
    }

    // Fulfill a method or connection request promise.
    template <class C>
    Status fulfill (uint32_t requestId, C result) {
        BOOST_LOG_NAMED_SCOPE("rpc::AsyncRequestManager::fulfill(result)");

        std::lock_guard<decltype(mPromisesMutex)> lock { mPromisesMutex };

        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            BOOST_LOG(mLog) << "request " << requestId << " does not exist";
            return Status::UNSOLICITED_REPLY;
        }

        auto promisePtr = util::get<std::promise<C>>(&iter->second);
        if (promisePtr) {
            BOOST_LOG(mLog) << "fulfilling request " << requestId;
            promisePtr->set_value(result);
        }
        else {
            // type mismatch
            BOOST_LOG(mLog) << "type mismatch fulfilling request " << requestId;
            auto eptr = std::make_exception_ptr(Error(Status::UNRECOGNIZED_RESULT));
            util::apply(Throw(eptr), iter->second);
        }

        mPromises.erase(iter);
        return Status::OK;
    }

    template <class C>
    Future<C> finalize (uint32_t requestId, Status status) {
        // If we ever decide to do anything with mPromises here, remember to
        // lock it with a std::lock_guard.
        printf("requestId %" PRId32 " wasted on %s\n", requestId, statusToString(status));

        throw Error(status);
    }

    template <class C>
    Future<C> finalize (uint32_t requestId) {
        BOOST_LOG_NAMED_SCOPE("rpc::AsyncRequestManager::finalize");
        std::promise<C>* promisePtr;
        {
            std::lock_guard<decltype(mPromisesMutex)> lock { mPromisesMutex };

            typename decltype(mPromises)::iterator iter;
            bool success;

            std::tie(iter, success) = tryMakePromise<C>(requestId);

            if (!success) {
                // this will break the existing promise
                mPromises.erase(iter);
                std::tie(iter, success) = tryMakePromise<C>(requestId);
                assert(success);
            }

            promisePtr = util::get<std::promise<C>>(&iter->second);
            assert(promisePtr);
        }

        BOOST_LOG(mLog) << "scheduling request " << requestId << " for reaping "
                        << "(mPromises:" << &mPromises << ")";
        mReaper.executeAfter(std::chrono::milliseconds(1000),
            [=] () {
                BOOST_LOG_NAMED_SCOPE("rpc::AsyncRequestManager::finalize::(lambda)");
                std::lock_guard<decltype(mPromisesMutex)> lock { mPromisesMutex };
                auto iter = mPromises.find(requestId);
                BOOST_LOG(mLog) << "request " << requestId
                                << (mPromises.end() == iter ? " no longer " : " still ")
                                << "exists (mPromises:" << &mPromises << ")";
                if (mPromises.end() != iter) {
                    auto promisePtr = util::get<std::promise<C>>(&iter->second);
                    assert(promisePtr);
                    auto eptr = std::make_exception_ptr(std::runtime_error(
                        std::string("No response to request ") +
                        std::to_string(requestId) +
                        std::string(" after 1000ms")));
                    BOOST_LOG(mLog) << "setting exception";
                    promisePtr->set_exception(eptr);
                    BOOST_LOG(mLog) << "erasing";
                    mPromises.erase(iter);
                }
            }
        );

        return promisePtr->get_future();
    }

    uint32_t nextRequestId () {
        return mNextRequestId++;
    }

private:
    mutable boost::log::sources::logger_mt mLog;

    boost::unordered_map
        < uint32_t
        , PromiseVariant
        > mPromises;
    std::mutex mPromisesMutex;

    template <class C>
    std::pair<typename decltype(mPromises)::iterator, bool>
    tryMakePromise (uint32_t requestId) {
        return mPromises.emplace(std::piecewise_construct,
                std::forward_as_tuple(requestId),
                std::forward_as_tuple(std::promise<C>()));
    }

    std::atomic<uint32_t> mNextRequestId = { 0 };

    // FIXME this means that every proxy will have its own reaper thread,
    // probably not what we want. Maybe there should be a globally accessible
    // rpc::reaper(), perhaps hidden in a .cpp, accessible only where we need
    // it.
    util::DeadlineScheduler mReaper;
};

template <class T, class Interface>
using AsyncProxy = Proxy<T, Interface, AsyncRequestManager>;

} // namespace rpc

#endif
