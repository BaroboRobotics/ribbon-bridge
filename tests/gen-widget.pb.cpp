#include "gen-widget.pb.hpp"
#include "rpc/def.hpp"

RPCDEF_CPP((barobo, Widget),
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
