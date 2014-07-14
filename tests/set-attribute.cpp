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

    auto future1 = widget.proxy().set(Attribute::attribute{14});
    auto future2 = widget.proxy().get(Attribute::attribute());

    try {
        future1.get(); // void
        std::cout << "set(attribute) succeeded\n";
        auto result = future2.get();
        assert(14 == result.value);
        std::cout << "get(attribute) returned: " << result.value << '\n';
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
