#ifndef barobo_Widget_INTERFACE
#define barobo_Widget_INTERFACE

/* GENERATED CODE */

#include "rpc/def.hpp"

#include "widget.pb.h"

#if 0
#define RPCDEF(interfaceNames, allAttributes, settableAttributes, subscribableAttributes, methods, broadcasts) \
    RPCDEF_FWD_DECL_INTERFACE(interfaceNames) \
    namespace rpc { \
    RPCDEF_FWD_DECL_isComponentFunctions(rpcdef_cat_scope(interfaceNames)) \
    }

RPCDEF((barobo, Widget),
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        ,
        (attribute)
        (noSubscriptionsAttribute)
        ,
        (attribute)
        (readonlyAttribute)
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
#endif

RPCDEF_FWD_DECL_INTERFACE((barobo, Widget))

namespace rpc {

RPCDEF_FWD_DECL_isComponentFunctions(barobo::Widget)

//////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE TRAITS

RPCDEF_Attribute((barobo, Widget),
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        )

RPCDEF_IsAttribute(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        (noSubscriptionsAttribute)
        (readonlyNoSubscriptionsAttribute)
        )

RPCDEF_IsSettableAttribute(barobo::Widget,
        (attribute)
        (noSubscriptionsAttribute)
        )

RPCDEF_IsSubscribableAttribute(barobo::Widget,
        (attribute)
        (readonlyAttribute)
        )

//////////////////////////////////////////////////////////////////////////////
// METHOD TRAITS

RPCDEF_MethodIn((barobo, Widget),
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

RPCDEF_MethodResult((barobo, Widget),
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

RPCDEF_ResultOf(barobo::Widget,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

RPCDEF_IsMethod(barobo::Widget,
        (nullaryNoResult)
        (nullaryWithResultOut)
        (nullaryWithResultError)
        (nullaryWithResult)
        (unaryNoResult)
        (unaryWithResult)
        (unaryWithResultOut)
        (unaryWithResultError)
        )

//////////////////////////////////////////////////////////////////////////////
// BROADCAST TRAITS

RPCDEF_Broadcast((barobo, Widget),
        (broadcast))

RPCDEF_IsBroadcast(barobo::Widget,
        (broadcast))

//////////////////////////////////////////////////////////////////////////////
// INTERFACE TRAITS

RPCDEF_Version((barobo, Widget), 0, 0, 0)

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
