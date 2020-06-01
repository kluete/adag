// DAG implementation

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>            // for std::max

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
    DAGImp(const size_t tot_nodes, const size_t num_root_nodes, const float slice_factor)
        : m_TotalNodes(tot_nodes), m_NumRootNodes(num_root_nodes), m_SliceFactor(slice_factor)
    {
        cout << "DAGImp() CTOR" << endl;
        cout << "  tot_nodes = " << tot_nodes << endl;
        cout << "  num_root_nodes = " << num_root_nodes << endl;
        cout << "  slice_factor = " << slice_factor << endl << endl;
        
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
        cout << "building DAG..." << endl << endl;
        
        m_NodeToChildNodesTab.resize(m_TotalNodes, {}/*empty child list*/);
        
        // slice size of downstream nodes
        const size_t    slice_size = m_TotalNodes * m_SliceFactor;
        
        // max (child) nodes in single branch
        const size_t    max_branch_nodes = (m_TotalNodes - m_NumRootNodes) / slice_size;
        
        auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
        
        size_t  max_node_children = 0;
        
        // per branch (1 branch = 1 thread)
        for (size_t thread = 0; thread < m_NumRootNodes; thread++)
        {
            size_t  walker_node = thread;
            
            cout << "  th[" << thread << "]:" << endl;
            
            // generate child nodes
            for (size_t j = 0; j < max_branch_nodes; j++)
            {
                const size_t  next_node = walker_node + (rnd_gen() * slice_size);
                
                assert(walker_node < m_TotalNodes);
                assert(next_node < m_TotalNodes);
                m_NodeToChildNodesTab[walker_node].push_back(next_node);
                
                max_node_children = std::max(max_node_children, m_NodeToChildNodesTab[walker_node].size());
                
                cout << "     node[" << j << "] : " << walker_node << " -> " << next_node << endl;
                
                walker_node = next_node;
            }
            
            // end branch
            m_NodeToChildNodesTab[walker_node].push_back(0);
        }
        
        cout << endl << "DAG done, max_node_children = " << max_node_children << endl << endl;
    }

    const size_t    m_TotalNodes;
    const size_t    m_NumRootNodes;
    const float     m_SliceFactor;
    
    vector<vector<size_t>>                  m_NodeToChildNodesTab;
    unordered_map<size_t, Actor::ActorId>   m_ActorIndexToIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

// static
IDag*    IDag::CreateDAG(const size_t tot_nodes, const size_t num_root_nodes, const float slice_factor)
{
    return new DAGImp(tot_nodes, num_root_nodes, slice_factor);
}

} // namespace zamai

// nada mas



