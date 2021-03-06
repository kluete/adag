
#pragma once

#include <vector>
#include "simplx.h"

namespace adag
{
// import into namespace
using std::vector;
using tredzone::Actor;

constexpr uint32_t  NODE_CHILD_END = std::numeric_limits<uint32_t>::max();

//---- DAG interface (a la Scott Meyers) ---------------------------------------

class IDag
{
public:

    virtual ~IDag() = default;
    
    virtual void                RegisterActorId(const uint32_t id, const Actor::ActorId &actor_id) = 0;
    virtual uint32_t            GetTotatlTerminations(void) const = 0;
    virtual vector<uint32_t>    GetChildNodes(const uint32_t id) const = 0;
    virtual Actor::ActorId      GetNodeActorId(const uint32_t id) const = 0;
    virtual void                DumpDAG(void) const = 0;
    
    static
    IDag*   CreateDAG(const uint32_t total_nodes, const uint32_t root_nodes, const uint32_t rnd_bucket_size);
};

} // namespace adag

// nada mas
