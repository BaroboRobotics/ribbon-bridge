#include "rpc/stdlibheaders.hpp"
#include "gen-widget.pb.hpp"
#include "rpc/message.hpp"
#include "rpc/def.hpp"

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

RPCDEF_getSubscriptionRecord(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (broadcast)
        )

} // namespace rpc
