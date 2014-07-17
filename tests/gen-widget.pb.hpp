#ifndef barobo_Widget_INTERFACE
#define barobo_Widget_INTERFACE

#include "rpc/def.hpp"
#include "widget.pb.h"

RPCDEF_HPP(
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

#endif
