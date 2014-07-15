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

    auto future = widget.proxy().subscribe(Attribute::attribute());

    try {
        future.get();
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    // Send a broadcast to the proxy
    widget.service().broadcast(Attribute::attribute{15});

    try {
        // Wait for the broadcast
        widget.proxy().unsubscribe(Attribute::attribute()).get();
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    assert(15 == widget.proxy().broadcastedAttribute().value);

    return testResult;
}
