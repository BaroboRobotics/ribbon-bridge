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
        widget.proxy().subscribe(Broadcast::broadcast()).get();
        auto status = widget.service().broadcast(Broadcast::broadcast{0.5});
        if (hasError(status)) { throw rpc::Error { statusToString(status) }; }
        widget.proxy().unsubscribe(Broadcast::broadcast()).get();
        assert(0.5 == widget.proxy().broadcastedBroadcast().value);
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
