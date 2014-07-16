#include "rpc/stdlibheaders.hpp"
#include "gen-widget.pb.hpp"
#include "rpc/message.hpp"
#include "rpc/def.hpp"

// .hpp
// XXX Attribute: all attributes
// XXX IsAttribute: all attributes
// XXX IsSettableAttribute: all settable attributes
// XXX IsSubscribableAttribute: all subscribable attributes
// XXX MethodIn: all methods
// XXX MethodResult: all methods
// XXX ResultOf: all methods
// XXX IsMethod: all methods
// XXX Broadcast: all broadcasts
// XXX IsBroadcast: all broadcasts
// XXX Version: major, minor, patch
// XXX ComponentInUnion: all non-readonly attributes, all methods
// XXX ComponentResultUnion: all attributes, all methods, all broadcasts
// XXX PromiseVariadic: all attributes, all methods
// XXX ComponentId: all components
// XXX componentId: all components
// XXX Subscriptions: all subscribable attributes, all broadcasts
// XXX GetInvoker: all attributes
// XXX SetInvoker: all settable attributes
// XXX FireInvoker: all methods
// XXX BroadcastInvoker: all subscribable attributes, all broadcasts
// XXX FulfillInvoker: all attributes, all methods


namespace rpc {

// all attributes
RPCDEF_pbFields_attributes(barobo_Widget_,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        )

// all methods
RPCDEF_pbFields_methods(barobo_Widget_,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

// all broadcasts
RPCDEF_pbFields_broadcasts(barobo_Widget_,
        (broadcast)
        )

// all attributes
RPCDEF_isAttribute(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        )

// all methods
RPCDEF_isMethod(barobo::Widget,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

// all broadcasts
RPCDEF_isBroadcast(barobo::Widget,
        (broadcast)
        )

// all settable attributes
RPCDEF_decodeSetPayload(barobo::Widget,
        (attribute)
        (noSubscriptionsAttribute)
        )

// all methods
RPCDEF_decodeFirePayload(barobo::Widget,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

// all subscribable attributes, all broadcasts
RPCDEF_decodeBroadcastPayload(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (broadcast)
        )

// all attributes, all methods
RPCDEF_decodeResultPayload(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

} // namespace rpc
