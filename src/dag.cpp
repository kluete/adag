// DAG implementation

#include <vector>
#include <unordered_map>
#include <random>

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
    DAGImp(const size_t max_nodes, const size_t max_node_fanning, const size_t num_entry_points, const size_t num_branching)
        : m_MaxNodes(max_nodes), m_MaxNodeFans(max_node_fanning), m_NbrStartNodes(num_entry_points),
        m_NodeToChildNodesTab(max_nodes)
    {
        CreateDAG();
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
    
    // generate index -> index graph
    void CreateDAG(void)
    {
        m_NodeToChildNodesTab.clear();
        
        auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
        (void)rnd_gen;
        
        for (size_t n = 0; n < m_NbrStartNodes; ++n)
        {
            
            for (size_t i = 0; i < m_NbrStartNodes; i++)
            {
                for (size_t j = m_NbrStartNodes; j < m_MaxNodes; j++)
                {
                
                }
            }
        }
        
        
    }

    const size_t    m_MaxNodes;
    const size_t    m_MaxNodeFans;
    const size_t    m_NbrStartNodes;
    
    vector<vector<size_t>>                  m_NodeToChildNodesTab;
    unordered_map<size_t, Actor::ActorId>   m_ActorIndexToIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

  
// static
IDag*    IDag::CreateDAG(const size_t max_nodes, const size_t max_node_fanning, const size_t num_threads, const size_t num_branching)
{
    return new DAGImp(max_nodes, max_node_fanning, num_threads, num_branching);
}

} // namespace zamai

// nada mas



