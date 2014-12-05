#include "widgetimpl.hpp"
#include "connectedrpcobject.hpp"

#include <iostream>

int main () {
    enum { SUCCEEDED, FAILED };
    int testResult = SUCCEEDED;

    using Broadcast = rpc::Broadcast<barobo::Widget>;

    ConnectedRpcObject<WidgetService, WidgetProxy> widget;

    try {
        auto status = widget.service().broadcast(Broadcast::broadcast{0.5});
        if (hasError(status)) { throw rpc::Error(status); }
        widget.proxy().connect().get(); // make sure we're synchronized
        auto value = widget.proxy().broadcastedBroadcast().value;
        assert(0.5 == value);
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
