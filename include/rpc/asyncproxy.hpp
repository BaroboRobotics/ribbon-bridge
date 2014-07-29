#ifndef RPC_ASYNCPROXY_HPP
#define RPC_ASYNCPROXY_HPP

#include "rpc/config.hpp"

#ifndef HAVE_STDLIB
#error "this file requires the standard library"
#endif

#include "rpc/error.hpp"
#include "rpc/proxy.hpp"

#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

#include <future>
#include <tuple>
#include <utility>

#include <cstdio>
#include <cinttypes>

namespace rpc {

template <class Interface>
class AsyncRequestManager {
    template <class... Ts>
    using MakePromiseVariant = boost::variant<std::promise<Ts>...>;

    using PromiseVariant = typename rpc::PromiseVariadic<Interface, MakePromiseVariant>::type;

    struct SetExceptionVisitor : boost::static_visitor<> {
        explicit SetExceptionVisitor (Status status) : mStatus(status) { }

        template <class P>
        void operator() (P& promise) const {
            printf("breaking hearts, %s\n", __PRETTY_FUNCTION__);
            Error error { statusToString(mStatus) };
            auto eptr = std::make_exception_ptr(error);
            promise.set_exception(eptr);
        }

    private:
        Status mStatus;
    };

public:
    template <class T>
    using Future = std::future<T>;

    Status fulfill (uint32_t requestId, Status status) {
        std::lock_guard<decltype(mPromisesMutex)> lock { mPromisesMutex };

        printf("requestId %" PRId32 " fulfilled with status %s\n", requestId,
                statusToString(status));

        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            return Status::UNSOLICITED_RESULT;
        }

        auto promisePtr = boost::get<std::promise<void>>(&iter->second);
        if (promisePtr) {
            if (!hasError(status)) {
                promisePtr->set_value();
            }
            else {
                boost::apply_visitor(SetExceptionVisitor { status }, iter->second);
            }
        }
        else {
            if (!hasError(status)) {
                // No error reported, but we have a type mismatch.
                printf("type mismatch with requestId %" PRId32 "\n", requestId);
                boost::apply_visitor(SetExceptionVisitor { Status::UNRECOGNIZED_RESULT }, iter->second);
            }
            else {
                boost::apply_visitor(SetExceptionVisitor { status }, iter->second);
            }
        }

        mPromises.erase(iter);
        return Status::OK;
    }

    template <class C>
    Status fulfill (uint32_t requestId, C result) {
        std::lock_guard<decltype(mPromisesMutex)> lock { mPromisesMutex };

        printf("requestId %" PRId32 " fulfilled with result\n", requestId);

        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            return Status::UNSOLICITED_RESULT;
        }

        auto promisePtr = boost::get<std::promise<C>>(&iter->second);
        if (promisePtr) {
            promisePtr->set_value(result);
        }
        else {
            // type mismatch
            printf("type mismatch with requestId %" PRId32 "\n", requestId);
            boost::apply_visitor(SetExceptionVisitor(Status::UNRECOGNIZED_RESULT), iter->second);
        }

        mPromises.erase(iter);
        return Status::OK;
    }

    template <class C>
    Future<C> finalize (uint32_t requestId, Status status) {
        // If we ever decide to do anything with mPromises here, remember to
        // lock it with a std::lock_guard.
        printf("requestId %" PRId32 " wasted on %s\n", requestId, statusToString(status));

        throw Error { statusToString(status) };
    }

    template <class C>
    Future<C> finalize (uint32_t requestId) {
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

            promisePtr = boost::get<std::promise<C>>(&iter->second);
            assert(promisePtr);
        }

        return promisePtr->get_future();
    }

    uint32_t nextRequestId () {
        return mNextRequestId++;
    }

private:
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
};

template <class T, class Interface>
using AsyncProxy = Proxy<T, Interface, AsyncRequestManager>;

} // namespace rpc

#endif
