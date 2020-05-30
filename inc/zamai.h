
#pragma once

#include "simplx.h"

namespace zamai
{
// import into namespace
using tredzone::Actor;

//---- DAG interface (a la Scott Meyers) ---------------------------------------

class IDag
{
public:

    virtual ~IDag() = default;
    
    virtual void    InitDAGNodes(void) = 0;
    virtual void    SetIndexActorId(const size_t &i, const Actor::ActorId &actor_id) = 0;
    virtual void    next(void) = 0;

    static
    IDag*   CreateDAG(const size_t max_total_nodes, const size_t max_node_fanning);
};

} // namespace zamai

// nada mas
