#include "gen-widget.pb.hpp"
#include "rpc/def.hpp"

RPCDEF_CPP((barobo, Widget),
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
