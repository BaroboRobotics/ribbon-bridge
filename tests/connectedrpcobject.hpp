#ifndef CONNECTEDRPCOBJECT_HPP
#define CONNECTEDRPCOBJECT_HPP

#include "fullduplextransport.hpp"

template <class S, class P>
class ConnectedRpcObject {
public:
    ConnectedRpcObject ()
            : mTransport()
            , mService(mTransport.replyPoster())
            , mProxy(mTransport.requestPoster()) {
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
    FullDuplexTransport mTransport;

    S mService;
    P mProxy;
};

#endif
