#include "widgetimpl.hpp"
#include "connectedrpcobject.hpp"

#include <iostream>

int main () {
    enum { SUCCEEDED, FAILED };
    int testResult = SUCCEEDED;

    using Method = rpc::MethodIn<barobo::Widget>;

    ConnectedRpcObject<WidgetService, WidgetProxy> widget;

    try {
        {
            auto result = widget.proxy().fire(Method::nullaryNoResult()).get();
            std::cout << "nullary with no result\n";
        }
        {
            auto result = widget.proxy().fire(Method::nullaryWithResult()).get();
            std::cout << "nullary with result: " << result.value << '\n';
        }
        {
            auto result = widget.proxy().fire(Method::unaryNoResult{0.5}).get();
            std::cout << "unary with no result\n";
        }
        {
            auto result = widget.proxy().fire(Method::unaryWithResult{0.5}).get();
            assert(0.5 == result.value);
            std::cout << "unary with result: " << result.value << '\n';
        }
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
