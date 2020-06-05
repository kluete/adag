
#pragma once

#include <vector>
#include "simplx.h"

namespace zamai
{
// import into namespace
using std::vector;
using tredzone::Actor;

constexpr size_t    TERMINATION_COUNT_BATCH = 1000;

//---- DAG interface (a la Scott Meyers) ---------------------------------------

class IDag
{
public:

    virtual ~IDag() = default;
    
    virtual void            RegisterIndexActorId(const size_t i, const Actor::ActorId &actor_id) = 0;
    virtual size_t          GetTotatlTerminations(void) const = 0;
    virtual vector<size_t>  GetChildNodes(const size_t i) const = 0;
    virtual Actor::ActorId  GetNodeActorId(const size_t i) const = 0;
    
    static
    IDag*   CreateDAG(const size_t total_nodes, const size_t root_nodes, const float rnd_slice_factor);
};

//---- Threaded Logger interface -----------------------------------------------

} // namespace zamai

// nada mas
