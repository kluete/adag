// DAG implementation

#include <vector>
#include <unordered_map>

#include "simplx.h"

#include "zamai.h"

namespace zamai
{
using namespace std;
using namespace tredzone;

//---- DAG imp -----------------------------------------------------------------

class DAGImp : public IDag
{
public:
    // ctor
    DAGImp(const size_t max_nodes, const size_t max_node_fanning, const int num_threads)
        : m_MaxNodes(max_nodes), m_MaxNodeFans(max_node_fanning),
        m_NodeToChildrenTab(max_nodes)
    {
        // generate index -> index graph
    }
    
    void    InitDAGNodes(void) override
    {
        
    }
    
    void    RegisterIndexActorId(const size_t &index, const Actor::ActorId &actor_id) override
    {
        assert(!m_ActorIndexToIdMap.count(index));
        
        m_ActorIndexToIdMap.insert({index, actor_id});
    }
   
    void    next(void) override
    {
        
    }

private:
    
    const size_t    m_MaxNodes;
    const size_t    m_MaxNodeFans;
    
    vector<vector<size_t>>                  m_NodeToChildrenTab;
    unordered_map<size_t, Actor::ActorId>   m_ActorIndexToIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

  
// static
IDag*    IDag::CreateDAG(const size_t max_nodes, const size_t max_node_fanning, const int num_threads)
{
    return new DAGImp(max_nodes, max_node_fanning, num_threads);
}

} // namespace zamai

// nada mas



