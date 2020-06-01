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
    DAGImp(const size_t max_nodes, const size_t max_node_fanning, const size_t num_entry_points, const float slice_factor)
        : m_MaxNodes(max_nodes), m_MaxNodeFans(max_node_fanning), m_NbrStartNodes(num_entry_points), m_SliceFactor(slice_factor),
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
        
        // slice size of downstream nodes
        const size_t    slice_size = m_MaxNodes * m_SliceFactor;
        (void)slice_size;
        
        // max (child) nodes in single branch
        const size_t    max_branch_nodes = m_MaxNodes * m_SliceFactor;
        
        auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
        (void)rnd_gen;
        
        // per branch
        for (size_t i = 0; i < m_NbrStartNodes; i++)
        {
            size_t  walker_node = i;
            
            // generate child nodes
            for (size_t j = 0; j < max_branch_nodes; j++)
            {
                const size_t  next_node = walker_node + (rnd_gen() * slice_size);
                
                assert(walker_node < m_MaxNodes);
                assert(next_node < m_MaxNodes);
                m_NodeToChildNodesTab[walker_node].push_back(next_node);
                
                walker_node = next_node;
            }
            
            // end branch
            m_NodeToChildNodesTab[walker_node].push_back(0);
        }
        
        
    }

    const size_t    m_MaxNodes;
    const size_t    m_MaxNodeFans;
    const size_t    m_NbrStartNodes;
    const float     m_SliceFactor;
    
    vector<vector<size_t>>                  m_NodeToChildNodesTab;
    unordered_map<size_t, Actor::ActorId>   m_ActorIndexToIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

  
// static
IDag*    IDag::CreateDAG(const size_t max_nodes, const size_t max_node_fanning, const size_t num_threads, const float slice_factor)
{
    return new DAGImp(max_nodes, max_node_fanning, num_threads, slice_factor);
}

} // namespace zamai

// nada mas



