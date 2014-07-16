#ifndef barobo_Widget_INTERFACE
#define barobo_Widget_INTERFACE

/* GENERATED CODE */

#include "rpc/def.hpp"

#include "widget.pb.h"

#define RPCDEF(interfaceNames, version, allAttributes, settableAttributes, subscribableAttributes, methods, broadcasts) \
    RPCDEF_FWD_DECL_INTERFACE(interfaceNames) \
    namespace rpc { \
    RPCDEF_Version(rpcdef_cat_scope(interfaceNames), version) \
    RPCDEF_FWD_DECL_isComponentFunctions(rpcdef_cat_scope(interfaceNames)) \
    RPCDEF_Attribute(interfaceNames, allAttributes) \
    RPCDEF_IsAttribute(rpcdef_cat_scope(interfaceNames), allAttributes) \
    RPCDEF_IsSettableAttribute(rpcdef_cat_scope(interfaceNames), settableAttributes) \
    RPCDEF_IsSubscribableAttribute(rpcdef_cat_scope(interfaceNames), subscribableAttributes) \
    RPCDEF_MethodIn(interfaceNames, methods) \
    RPCDEF_MethodResult(interfaceNames, methods) \
    RPCDEF_ResultOf(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_IsMethod(rpcdef_cat_scope(interfaceNames), methods) \
    RPCDEF_Broadcast(interfaceNames, broadcasts) \
    RPCDEF_IsBroadcast(rpcdef_cat_scope(interfaceNames), broadcasts) \
    RPCDEF_ComponentInUnion(rpcdef_cat_scope(interfaceNames), settableAttributes, methods) \
    }

RPCDEF(
        // the interface we're defining and it's version triplet
        (barobo, Widget), (0, 0, 0),
        // all attributes
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        ,
        // all settable attributes
        (attribute)
        (noSubscriptionsAttribute)
        ,
        // all subscribable attributes
        (attribute)
        (readonlyAttribute)
        ,
        // all methods
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        ,
        // all broadcasts
        (broadcast)
        )

namespace rpc {
#if 0
// Settable attributes and methods
RPCDEF_ComponentInUnion(barobo::Widget,
        (attribute)
        (noSubscriptionsAttribute)
        ,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )
#endif

// All components
RPCDEF_ComponentResultUnion(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        ,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        ,
        (broadcast)
        )

// All attributes, all methods
RPCDEF_PromiseVariadic(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        ,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

/* COMPONENT IDs */

RPCDEF_ComponentId(barobo::Widget,
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
        (broadcast)
        )

RPCDEF_componentId(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        ,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        ,
        (broadcast)
        )

RPCDEF_Subscriptions(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (broadcast)
        )

RPCDEF_GetInvoker(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        )

RPCDEF_SetInvoker(barobo::Widget,
        (attribute)
        (noSubscriptionsAttribute)
        )

RPCDEF_FireInvoker(barobo::Widget,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

RPCDEF_BroadcastInvoker(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (broadcast)
        )

// All attributes, all methods
RPCDEF_FulfillInvoker(barobo::Widget,
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

#endif
