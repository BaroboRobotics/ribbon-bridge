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

    void post (const BufferType& buffer) {
        mPostFunc(buffer);
    }

    /* These typedefs aren't required, but it makes things more readable. If
     * you implement multiple interfaces, you might make multiple typedefs. */
    using MethodIn = rpc::MethodIn<barobo::Widget>;
    using MethodResult = rpc::MethodResult<barobo::Widget>;
    using Attribute = rpc::Attribute<barobo::Widget>;

    Attribute::attribute onGet (Attribute::attribute) {
        return attribute;
    }
    void onSet (Attribute::attribute args) {
        attribute = args;
        broadcast(args);
    }

    Attribute::readonlyAttribute onGet (Attribute::readonlyAttribute) {
        return readonlyAttribute;
    }
    // XXX but note setReadonlyAttribute below

    Attribute::noSubscriptionsAttribute onGet (Attribute::noSubscriptionsAttribute) {
        return noSubscriptionsAttribute;
    }
    void onSet (Attribute::noSubscriptionsAttribute args) {
        noSubscriptionsAttribute = args;
        // broadcast not called
    }

    Attribute::readonlyNoSubscriptionsAttribute onGet (Attribute::readonlyNoSubscriptionsAttribute) {
        return readonlyNoSubscriptionsAttribute;
    }

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
    /* Certain readonly attributes will need updating by the service itself,
     * like accelerometer readings. Since readonlyAttribute is subscribable,
     * any privileged set function must remember to broadcast the update. */
    void setReadonlyAttribute (Attribute::readonlyAttribute val) {
        readonlyAttribute = val;
        broadcast(val);
    }

    Attribute::attribute attribute = { 13 };
    Attribute::readonlyAttribute readonlyAttribute = { 13 };
    Attribute::noSubscriptionsAttribute noSubscriptionsAttribute = { 13 };
    Attribute::readonlyNoSubscriptionsAttribute readonlyNoSubscriptionsAttribute = { 13 };

    std::function<void(const BufferType&)> mPostFunc;
};

class WidgetProxy : public rpc::AsyncProxy<WidgetProxy, barobo::Widget> {
public:
    WidgetProxy (std::function<void(const BufferType&)> postFunc) : mPostFunc(postFunc) { }

    void post (const BufferType& buffer) {
        mPostFunc(buffer);
    }

    using Attribute = rpc::Attribute<barobo::Widget>;
    using Broadcast = rpc::Broadcast<barobo::Widget>;

    void onBroadcast (Attribute::attribute args) {
        printf("onBroadcast(attribute): %" PRId32 "\n", args.value);
        mBroadcastedAttribute = args;
    }

    void onBroadcast (Attribute::readonlyAttribute args) {
        printf("onBroadcast(readonlyAttribute): %" PRId32 "\n", args.value);
        mBroadcastedReadonlyAttribute = args;
    }

    void onBroadcast (Broadcast::broadcast args) {
        printf("onBroadcast(broadcast): %f\n", args.value);
        mBroadcastedBroadcast = args;
    }

    //////

    Attribute::attribute broadcastedAttribute () const {
        return mBroadcastedAttribute;
    }

    Attribute::readonlyAttribute broadcastedReadonlyAttribute () const {
        return mBroadcastedReadonlyAttribute;
    }

    Broadcast::broadcast broadcastedBroadcast () const {
        return mBroadcastedBroadcast;
    }

private:
    std::function<void(const BufferType&)> mPostFunc;

    Attribute::attribute mBroadcastedAttribute;
    Attribute::readonlyAttribute mBroadcastedReadonlyAttribute;
    Broadcast::broadcast mBroadcastedBroadcast;
};

#endif
