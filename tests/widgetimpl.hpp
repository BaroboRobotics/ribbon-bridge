#ifndef RPC_TESTS_WIDGETIMPL_HPP
#define RPC_TESTS_WIDGETIMPL_HPP

#include "rpc/service.hpp"
#include "rpc/asyncproxy.hpp"

/* You first need to include the generated interface code. */
#include "gen-widget.pb.hpp"

/* Implementation of the barobo::Widget interface. */

class WidgetService : public rpc::Service<WidgetService, barobo::Widget> {
public:
    WidgetService (std::function<void(const BufferType&)> postFunc) : mPostFunc(postFunc) { }

    void bufferToProxy (const BufferType& buffer) {
        mPostFunc(buffer);
    }

    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using MethodIn = rpc::MethodIn<barobo::Widget>;
    using MethodResult = rpc::MethodResult<barobo::Widget>;

    MethodResult::nullaryNoResult onFire (MethodIn::nullaryNoResult) {
        MethodResult::nullaryNoResult result;
        memset(&result, 0, sizeof(result));
        return result;
    }

    MethodResult::nullaryWithResultOut onFire (MethodIn::nullaryWithResultOut) {
        MethodResult::nullaryWithResultOut result;
        memset(&result, 0, sizeof(result));
        result.has_out = true;
        result.out.value = 2.718281828;
        return result;
    }

    MethodResult::nullaryWithResultError onFire (MethodIn::nullaryWithResultError) {
        MethodResult::nullaryWithResultError result;
        memset(&result, 0, sizeof(result));
        result.has_error = true;
        result.error.value = barobo_Widget_nullaryWithResultError_Result_Error_Value_FAILURE;
        return result;
    }

    MethodResult::nullaryWithResult onFire (MethodIn::nullaryWithResult) {
        MethodResult::nullaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.has_out = true;
        result.out.value = 2.718281828;
        return result;
    }

    MethodResult::unaryNoResult onFire (MethodIn::unaryNoResult args) {
        MethodResult::unaryNoResult result;
        memset(&result, 0, sizeof(result));
        printf("onFire(unaryNoResult): %f\n", args.value);
        return result;
    }

    MethodResult::unaryWithResultOut onFire (MethodIn::unaryWithResultOut args) {
        MethodResult::unaryWithResultOut result;
        memset(&result, 0, sizeof(result));
        result.has_out = true;
        result.out.value = args.value;
        return result;
    }

    MethodResult::unaryWithResultError onFire (MethodIn::unaryWithResultError args) {
        MethodResult::unaryWithResultError result;
        memset(&result, 0, sizeof(result));
        printf("onFire(unaryWithResultError): %f\n", args.value);
        result.has_error = true;
        result.error.value = barobo_Widget_unaryWithResultError_Result_Error_Value_FAILURE;
        return result;
    }

    MethodResult::unaryWithResult onFire (MethodIn::unaryWithResult args) {
        MethodResult::unaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.has_out = true;
        result.out.value = args.value;
        return result;
    }

private:
    std::function<void(const BufferType&)> mPostFunc;
};

class WidgetProxy : public rpc::AsyncProxy<WidgetProxy, barobo::Widget> {
public:
    WidgetProxy (std::function<void(const BufferType&)> postFunc) : mPostFunc(postFunc) { }

    void bufferToService (const BufferType& buffer) {
        mPostFunc(buffer);
    }

    using Broadcast = rpc::Broadcast<barobo::Widget>;

    void onBroadcast (Broadcast::broadcast args) {
        printf("onBroadcast(broadcast): %f\n", args.value);
        mBroadcastedBroadcast = args;
    }

    //////

    Broadcast::broadcast broadcastedBroadcast () const {
        return mBroadcastedBroadcast;
    }

private:
    std::function<void(const BufferType&)> mPostFunc;

    Broadcast::broadcast mBroadcastedBroadcast;
};

#endif
