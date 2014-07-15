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
        widget.proxy().set(Attribute::attribute{14}).get();
        std::cout << "set(attribute) succeeded\n";
        auto result = widget.proxy().get(Attribute::attribute()).get();
        assert(14 == result.value);
        std::cout << "get(attribute) returned: " << result.value << '\n';
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
