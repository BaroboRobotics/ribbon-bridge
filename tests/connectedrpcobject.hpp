#ifndef CONNECTEDRPCOBJECT_HPP
#define CONNECTEDRPCOBJECT_HPP

#include "fullduplextransport.hpp"

template <class S, class P>
class ConnectedRpcObject {
public:
    ConnectedRpcObject ()
            : mService(mTransport.replyPoster())
            , mProxy(mTransport.requestPoster())
            , mTransport() {
        mTransport.onRequest([this] (const rpc::Buffer<256>& payload) {
            auto success = mService.deliver(payload);
            assert(!hasError(success));
        });
        mTransport.onReply([this] (const rpc::Buffer<256>& payload) {
            auto success = mProxy.deliver(payload);
            assert(!hasError(success));
        });
    }

    S& service () { return mService; }
    P& proxy () { return mProxy; }

private:
    S mService;
    P mProxy;

    // members are destroyed in reverse order, and since mTransport contains
    // closures with access to mService and mProxy, they must be destroyed
    // *after* mTransport, otherwise kaboom
    FullDuplexTransport mTransport;
};

#endif
