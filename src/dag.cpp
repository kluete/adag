// DAG implementation

#include "zamai.h"

namespace zamai
{
//---- DAG imp -----------------------------------------------------------------

class DAGImp : public IDag
{
public:
    // ctor
    DAGImp
    {
        
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



