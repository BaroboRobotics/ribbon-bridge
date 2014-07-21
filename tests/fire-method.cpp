#include "widgetimpl.hpp"
#include "connectedrpcobject.hpp"

#include <iostream>

int main () {
    enum { SUCCEEDED, FAILED };
    int testResult = SUCCEEDED;

    using Attribute = rpc::Attribute<barobo::Widget>;
    using Method = rpc::MethodIn<barobo::Widget>;
    using Broadcast = rpc::Broadcast<barobo::Widget>;

    ConnectedRpcObject<WidgetService, WidgetProxy> widget;

    try {
        {
            auto result = widget.proxy().fire(Method::nullaryNoResult()).get();
            assert(!result.has_out && !result.has_error);
            std::cout << "nullary with no result\n";
        }
        {
            auto result = widget.proxy().fire(Method::nullaryWithResultOut()).get();
            assert(result.has_out && !result.has_error);
            std::cout << "nullary with result out: " << result.out.value << '\n';
        }
        {
            auto result = widget.proxy().fire(Method::nullaryWithResultError()).get();
            assert(!result.has_out && result.has_error);
            assert(barobo_Widget_nullaryWithResultError_Result_Error_Value_FAILURE == result.error.value);
            std::cout << "nullary with result error\n";
        }
        {
            auto result = widget.proxy().fire(Method::nullaryWithResult()).get();
            assert(result.has_out && !result.has_error);
            std::cout << "nullary with result out/error: " << result.out.value << '\n';
        }
        {
            auto result = widget.proxy().fire(Method::unaryNoResult{0.5}).get();
            assert(!result.has_out && !result.has_error);
            std::cout << "unary with no result\n";
        }
        {
            auto result = widget.proxy().fire(Method::unaryWithResultOut{0.5}).get();
            assert(result.has_out && !result.has_error);
            // 0.5 should be perfectly representable by IEEE754
            assert(0.5 == result.out.value);
            std::cout << "unary with result out: " << result.out.value << '\n';
        }
        {
            auto result = widget.proxy().fire(Method::unaryWithResultError{0.5}).get();
            assert(!result.has_out && result.has_error);
            assert(barobo_Widget_unaryWithResultError_Result_Error_Value_FAILURE == result.error.value);
            std::cout << "unary with result error\n";
        }
        {
            auto result = widget.proxy().fire(Method::unaryWithResult{0.5}).get();
            assert(result.has_out && !result.has_error);
            assert(0.5 == result.out.value);
            std::cout << "unary with result out/error: " << result.out.value << '\n';
        }
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
