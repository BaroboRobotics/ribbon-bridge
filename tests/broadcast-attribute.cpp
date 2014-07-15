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
        // It's possible to broadcast without actually setting the value.
        widget.proxy().subscribe(Attribute::attribute()).get();
        auto status = widget.service().broadcast(Attribute::attribute{15});
        if (hasError(status)) { throw rpc::Error { statusToString(status) }; }
        widget.proxy().unsubscribe(Attribute::attribute()).get();
        assert(15 == widget.proxy().broadcastedAttribute().value);

        // And of course possible to do it the normal way.
        widget.proxy().subscribe(Attribute::attribute()).get();
        widget.proxy().set(Attribute::attribute{16}).get();
        widget.proxy().unsubscribe(Attribute::attribute()).get();
        assert(16 == widget.proxy().broadcastedAttribute().value);

        // For good measure
        assert(16 == widget.proxy().get(Attribute::attribute()).get().value);
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
