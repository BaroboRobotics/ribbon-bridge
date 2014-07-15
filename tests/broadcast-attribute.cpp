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
        std::cout << "Testing attribute broadcast ...\n";
        // It's possible to broadcast without actually setting the value.
        widget.proxy().subscribe(Attribute::attribute()).get();
        auto status = widget.service().broadcast(Attribute::attribute{15});
        if (hasError(status)) { throw rpc::Error { statusToString(status) }; }
        widget.proxy().unsubscribe(Attribute::attribute()).get();
        assert(15 == widget.proxy().broadcastedAttribute().value);
        std::cout << "Passed (once)\n";

        // And of course possible to do it the normal way.
        widget.proxy().subscribe(Attribute::attribute()).get();
        widget.proxy().set(Attribute::attribute{16}).get();
        widget.proxy().unsubscribe(Attribute::attribute()).get();
        assert(16 == widget.proxy().broadcastedAttribute().value);
        std::cout << "Passed (twice)\n";

        // For good measure
        assert(16 == widget.proxy().get(Attribute::attribute()).get().value);
        std::cout << "Passed (thrice)\n";

        std::cout << "Testing readonlyAttribute broadcast ...\n";
        // For readonly attributes, the only way to trigger their update
        // broadcasts is by calling .broadcast() on the service.
        widget.proxy().subscribe(Attribute::readonlyAttribute()).get();
        status = widget.service().broadcast(Attribute::readonlyAttribute{17});
        if (hasError(status)) { throw rpc::Error { statusToString(status) }; }
        widget.proxy().unsubscribe(Attribute::readonlyAttribute()).get();
        assert(17 == widget.proxy().broadcastedReadonlyAttribute().value);
        std::cout << "Passed\n";
    }
    catch (const rpc::Error& exc) {
        std::cout << "RPC error: " << exc.what() << '\n';
        testResult = FAILED;
    }

    return testResult;
}
