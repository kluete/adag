// DAG implementation

#include <vector>

#include "simplx.h"

#include "zamai.h"

namespace zamai
{
using namespace tredzone;
    
//---- DAG imp -----------------------------------------------------------------

class DAGImp : public IDag
{
public:
    // ctor
    DAGImp(const size_t max_nodes, const size_t max_node_fanning)
    {
        // generate index -> index graph
    }
    
    void    SetIndexActorId(const size_t &i, const Actor::ActorId &actor_id) override
    {
        (void)i;
        (void)actor_id;
    }
   
    void    next(void) override
    {
        
    }

private:
    
    
};

//---- INSTANTIATION -----------------------------------------------------------

  
// static
IDag*    IDag::CreateDAG(const size_t max_nodes, const size_t max_node_fanning)
{
    return new DAGImp(max_nodes, max_node_fanning);
}

} // namespace zamai

// nada mas



