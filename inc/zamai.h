
#pragma once

#include "simplx.h"

namespace zamai
{
// import into namespace
using tredzone::Actor;
    
class IDag
{
public:

    virtual ~IDag() = default;
    
    virtual void    SetIndexActorId(const size_t &i, const Actor::ActorId &actor_id) = 0;
    virtual void    next(void) = 0;

    static
    IDag*   CreateDAG(const size_t max_nodes);
};

} // namespace zamai

// nada mas
