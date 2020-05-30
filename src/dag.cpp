// DAG implementation

#include "zamai.h"

#include "simplx.h"

namespace zamai
{
using namespace tredzone;
    
//---- DAG imp -----------------------------------------------------------------

class DAGImp : public IDag
{
public:
    // ctor
    DAGImp(const size_t max_nodes)
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
IDag*    IDag::CreateDAG(const size_t max_nodes)
{
    return new DAGImp(max_nodes);
}

} // namespace zamai

// nada mas



