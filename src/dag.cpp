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
    DAGImp(const size_t total_nodes, const size_t root_nodes, const float rnd_slice_factor)
        : m_TotalNodes(total_nodes), m_RootNodes(root_nodes), m_RndSliceFactor(rnd_slice_factor)
    {
        cout << "DAGImp() CTOR" << endl;
        cout << "  total_nodes = " << total_nodes << endl;
        cout << "  root_nodes = " << root_nodes << endl;
        cout << "  rnd_slice_factor = " << rnd_slice_factor << endl << endl;
        
        CreateDAG();
    }
    
    void    RegisterIndexActorId(const size_t i, const Actor::ActorId &actor_id) override
    {
        assert(i < m_TotalNodes);
        
        if (m_ActorIndexToIdMap.count(i))
        {
            cout << "index " << i << " was already registered!!!" << endl;
            
            return;
        }
        
        // make sure wasn't already computed
        assert(!m_ActorIndexToIdMap.count(i));
        
        m_ActorIndexToIdMap.insert({i, actor_id});
    }
   
    bool     IsIndexRegistered(const size_t i) const override
    {
        assert(i < m_TotalNodes);
        
        const bool  f = m_ActorIndexToIdMap.count(i);
        
        return f;
    }
    
    size_t    GetNumRegisteredIndices(void) const override
    {
        const size_t    n = m_ActorIndexToIdMap.size();
        
        return n;
    }
    
    vector<size_t>  GetChildNodes(const size_t i) override
    {
        assert(i < m_TotalNodes);
        
        // make sure exists (though may be empty)
        assert(m_ActorIndexToIdMap.count(i));
        
        const vector<size_t> children = m_NodeToChildNodesTab.at(i);
        
        return children;
    }
    
    Actor::ActorId  GetNodeActorId(const size_t i) override
    {
        assert(i < m_TotalNodes);
        
        if (!m_ActorIndexToIdMap.count(i))
        {
            const size_t    n_reg = GetNumRegisteredIndices();
            
            cout << "actor index " << i << " doesn't have an actor ID (out of total " << n_reg << " registered)" << endl;
            
            assert(false);
        }
        
        // make sure exists (though may be empty actor ID)
        assert(m_ActorIndexToIdMap.count(i));
        
        const Actor::ActorId    aid = m_ActorIndexToIdMap.at(i);
        // check isn't null actor id
        assert(aid != Actor::ActorId());
        
        return aid;
    }


private:
    
    // generate index -> index graph
    void CreateDAG(void)
    {
        cout << "building DAG..." << endl << endl;
        
        m_NodeToChildNodesTab.resize(m_TotalNodes, {}/*empty child list*/);
        
        // slice size of downstream nodes
        const size_t    slice_size = m_TotalNodes * m_RndSliceFactor;
        
        // max (child) nodes in single branch
        const size_t    max_branch_nodes = (m_TotalNodes - m_RootNodes) / slice_size;
        
        auto	rnd_gen = bind(uniform_int_distribution<>(1, slice_size - 1), default_random_engine{0/*seed*/});
        
        size_t  max_node_children = 0;
        
        // per branch (1 branch = 1 thread)
        for (size_t thread = 0; thread < m_RootNodes; thread++)
        {
            size_t  walker_node = thread;
            
            cout << "  th[" << thread << "]:" << endl;
            
            // generate child nodes
            for (size_t j = 0; j < max_branch_nodes; j++)
            {
                const size_t  next_node = walker_node + rnd_gen();
                
                assert(walker_node < m_TotalNodes);
                assert(next_node < m_TotalNodes);
                assert(walker_node != next_node);       // don't loop over self
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
    const size_t    m_RootNodes;
    const float     m_RndSliceFactor;
    
    vector<vector<size_t>>                  m_NodeToChildNodesTab;
    unordered_map<size_t, Actor::ActorId>   m_ActorIndexToIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

// static
IDag*    IDag::CreateDAG(const size_t total_nodes, const size_t root_nodes, const float rnd_slice_factor)
{
    return new DAGImp(total_nodes, root_nodes, rnd_slice_factor);
}

} // namespace zamai

// nada mas



