#ifndef RPC_ASYNCPROXY_HPP
#define RPC_ASYNCPROXY_HPP

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

namespace rpc {

template <class T, class Interface>
class AsyncProxy : public Proxy<AsyncProxy<T, Interface>, Interface, std::future> {
    template <class... Ts>
    using MakePromiseVariant = boost::variant<std::promise<Ts>...>;

    using PromiseVariant = typename rpc::PromiseVariadic<Interface, MakePromiseVariant>::type;

    struct SetExceptionVisitor : boost::static_visitor<> {
        explicit SetExceptionVisitor (Status status) : mStatus(status) { }

        template <class P>
        void operator() (P& promise) const {
            Error error { statusToString(mStatus) };
            auto eptr = std::make_exception_ptr(error);
            promise.set_exception(eptr);
        }

    private:
        Status mStatus;
    };

public:
    using BufferType = typename rpc::Proxy<AsyncProxy<T, Interface>, Interface, std::future>::BufferType;

    template <class C>
    void broadcast (C args, ONLY_IF(IsAttribute<C>::value || IsBroadcast<C>::value)) {
        static_cast<T*>(this)->broadcast(args);
    }

    Status fulfill (uint32_t requestId, Status status) {
        printf("requestId %" PRId32 " fulfilled with status %s\n", requestId,
                statusToString(status));

        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            return Status::UNSOLICITED_RESULT;
        }

        auto promisePtr = boost::get<std::promise<void>>(&iter->second);
        if (promisePtr && !hasError(status)) {
            promisePtr->set_value();
        }
        else if (!hasError(status)) {
            // No error reported, but we have a type mismatch.
            boost::apply_visitor(SetExceptionVisitor { Status::UNRECOGNIZED_RESULT }, iter->second);
        }
        else {
            boost::apply_visitor(SetExceptionVisitor { status }, iter->second);
        }

        mPromises.erase(iter);
        return Status::OK;
    }

    template <class C>
    Status fulfill (uint32_t requestId, C result) {
        printf("requestId %" PRId32 " fulfilled with some shit\n", requestId);

        auto iter = mPromises.find(requestId);
        if (mPromises.end() == iter) {
            return Status::UNSOLICITED_RESULT;
        }

        auto promisePtr = boost::get<std::promise<C>>(&iter->second);
        if (!promisePtr) {
            // type mismatch
            boost::apply_visitor(SetExceptionVisitor(Status::UNRECOGNIZED_RESULT), iter->second);
        }
        else {
            promisePtr->set_value(result);
        }

        mPromises.erase(iter);
        return Status::OK;
    }

    template <class C>
    std::future<C> finalize (uint32_t requestId, Status status) {
        return { };// FIXME
    }

    template <class C>
    std::future<C> finalize (uint32_t requestId, const BufferType& buffer) {
        typename decltype(mPromises)::iterator iter;
        bool success;

        std::tie(iter, success) = tryMakePromise<C>(requestId);

        if (!success) {
            // this will break the existing promise
            mPromises.erase(iter);
            std::tie(iter, success) = tryMakePromise<C>(requestId);
            assert(success);
        }

        auto promisePtr = boost::get<std::promise<C>>(&iter->second);
        assert(promisePtr);

        /* We must call get_future() before post(). If we call post first,
         * there is a chance that the implementor might run with the buffer,
         * deliver it to the service, deliver the response back to us, fulfill
         * the promise, erasing it, and thus invalidating our promise pointer,
         * all before we get its future. */
        auto future = promisePtr->get_future();
        static_cast<T*>(this)->post(buffer);
        return future;
    }

private:
    boost::unordered_map
        < uint32_t
        , PromiseVariant
        > mPromises;

    template <class C>
    std::pair<typename decltype(mPromises)::iterator, bool>
    tryMakePromise (uint32_t requestId) {
        return mPromises.emplace(std::piecewise_construct,
                std::forward_as_tuple(requestId),
                std::forward_as_tuple(std::promise<C>()));
    }
};

} // namespace rpc

#endif
