
#pragma once

#include <vector>
#include "simplx.h"

namespace zamai
{
// import into namespace
using std::vector;
using tredzone::Actor;

//---- DAG interface (a la Scott Meyers) ---------------------------------------

class IDag
{
public:

    virtual ~IDag() = default;
    
    virtual void            RegisterIndexActorId(const size_t &i, const Actor::ActorId &actor_id) = 0;
    virtual vector<size_t>  GetChildNodes(const size_t & i) = 0;

    static
    IDag*   CreateDAG(const size_t total_nodes, const size_t num_root_nodes, const float slice_factor);
};

} // namespace zamai

// nada mas
