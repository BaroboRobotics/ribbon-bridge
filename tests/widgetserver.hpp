#ifndef RPC_TESTS_WIDGETIMPL_HPP
#define RPC_TESTS_WIDGETIMPL_HPP

#include "rpc/server.hpp"

/* You first need to include the generated interface code. */
#include "gen-widget.pb.hpp"

/* Implementation of the barobo::Widget interface. */

class WidgetServer : public rpc::Server<WidgetServer, barobo::Widget> {
public:
    WidgetServer (std::function<void(const BufferType&)> postFunc) : mPostFunc(postFunc) { }

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

    MethodResult::nullaryWithResult onFire (MethodIn::nullaryWithResult) {
        MethodResult::nullaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.value = 2.718281828;
        return result;
    }

    MethodResult::unaryNoResult onFire (MethodIn::unaryNoResult args) {
        MethodResult::unaryNoResult result;
        memset(&result, 0, sizeof(result));
        printf("onFire(unaryNoResult): %f\n", args.value);
        return result;
    }

    MethodResult::unaryWithResult onFire (MethodIn::unaryWithResult args) {
        MethodResult::unaryWithResult result;
        memset(&result, 0, sizeof(result));
        result.value = args.value;
        return result;
    }

private:
    std::function<void(const BufferType&)> mPostFunc;
};

#endif
