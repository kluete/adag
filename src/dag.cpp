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
    DAGImp()
    {
        
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
IDag*    IDag::CreateDAG(void)
{
    return new DAGImp();
}

} // namespace zamai

// nada mas



