// DAG implementation

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <thread>
#include <mutex>
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
    DAGImp(const size_t total_nodes, const size_t root_nodes, const size_t rnd_bucket_size)
        : m_TotalNodes(total_nodes), m_RootNodes(root_nodes), m_RndBucketSize(rnd_bucket_size),
        m_MaxBranchNodes((m_TotalNodes - m_RootNodes) / m_RndBucketSize),
        m_TraversedNodes(0), m_TotalTerminations(0)
    {
        assert(m_MaxBranchNodes > 0);
        
        cout << "DAGImp() CTOR" << endl;
        cout << "  m_MaxBranchNodes = " << m_MaxBranchNodes << endl;
        
        CreateDAG();
        
        // calc total path terminations
        m_TotalTerminations = CalcTotalTerminations();
    }
    
    size_t    GetTotatlTerminations(void) const override
    {
        return m_TotalTerminations;
    }
    
    void    RegisterIndexActorId(const size_t i, const Actor::ActorId &actor_id) override
    {
        assert(i < m_TotalNodes);
        
        // check wasn't already registered
        assert(!m_ActorIndexToIdMap.count(i));
        
        // make sure wasn't already computed
        assert(!m_ActorIndexToIdMap.count(i));
        
        m_ActorIndexToIdMap.insert({i, actor_id});
    }
   
    vector<size_t>  GetChildNodes(const size_t i) const override
    {
        assert(i < m_TotalNodes);
        
        // mode always exists (though may be empty)
        
        const vector<size_t>    children = m_NodeToChildNodesTab.at(i);
        
        return children;
    }
    
    Actor::ActorId  GetNodeActorId(const size_t i) const override
    {
        assert(i < m_TotalNodes);
        
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
        
        assert(m_RndBucketSize > 1);
        
        auto	rnd_gen = bind(uniform_int_distribution<>(1, m_RndBucketSize - 1), default_random_engine{0/*seed*/});
        
        size_t  max_node_children = 0;
        
        // per branch (1 branch = 1 thread)
        for (size_t thread = 0; thread < m_RootNodes; thread++)
        {
            size_t  walker_node = thread;
            
            cout << "  th[" << thread << "]:" << endl;
            
            // generate child nodes
            for (size_t j = 0; j < m_MaxBranchNodes; j++)
            {
                const size_t  offset = rnd_gen() + (j == 0 ? m_RootNodes : 0);       // prevent root nodes to burrow into one another
                assert(offset > 0);
                const size_t  next_node = walker_node + offset;
                
                assert(walker_node < m_TotalNodes);
                assert(next_node < m_TotalNodes);
                assert(walker_node < next_node);       // don't loop over self or upstream
                
                m_NodeToChildNodesTab[walker_node].push_back(next_node);
                
                // compute any node's manimum # of children
                max_node_children = std::max(max_node_children, m_NodeToChildNodesTab[walker_node].size());
                
                // cout << "     node[" << j << "] : " << walker_node << " -> " << next_node << endl;
                
                walker_node = next_node;
            }
            
            // end branch
            m_NodeToChildNodesTab[walker_node].push_back(0);
            
             cout << endl << "DAG done, max_node_children = " << max_node_children << endl << endl;
        }
    }
    
    // calc total path terminations (recursively)
    void    CalcPathTerminations(const size_t node, size_t &n_path_nodes, const int depth) const
    {
        //assert(!m_VisitedNodeSet.count(node));
        // m_VisitedNodeSet.insert(node);
        
        m_TraversedNodes++;
        if (0 == m_TraversedNodes % TERMINATION_COUNT_BATCH)
        {
                cout << " count-traversed nodes = " << m_TraversedNodes << endl;
        }
        
        const vector<size_t>    child_nodes = GetChildNodes(node);
        if (child_nodes.empty())
        {
            // no child nodes
            n_path_nodes++;
            return;
        }
        
        // send to child nodes
        for (const size_t child_id: child_nodes)
        {
            if (child_id == 0)
            {
                // end node marker
                n_path_nodes++;
                return;
            }
            
            assert(child_id > node);
  
            // RECURSE
            CalcPathTerminations(child_id, n_path_nodes, depth + 1);
        }
    }
    
    size_t  CalcTotalTerminations(void) const
    {
        cout << "calculating total # of terminations..." << endl;
        
        size_t  n_endings  = 0;
        
        for (size_t thread = 0; thread < m_RootNodes; thread++)
        {
            // clear per-branch hashset
            m_VisitedNodeSet.clear();
            
            size_t  walker_node = thread;
            (void)walker_node;
            
            size_t  n_path_nodes = 0;
            
            CalcPathTerminations(walker_node, n_path_nodes/*&*/, 0/*depth*/);
            
            n_endings += n_path_nodes;
        }

        cout << "  DAG has " << n_endings << " total # of terminations..." << endl;
        
        return n_endings;
    }

    const size_t    m_TotalNodes;
    const size_t    m_RootNodes;
    const size_t    m_RndBucketSize;
    const size_t    m_MaxBranchNodes;
    
mutable size_t          m_TraversedNodes;
mutable unordered_set<size_t>   m_VisitedNodeSet;               // (per branch)
    size_t          m_TotalTerminations;
    
    vector<vector<size_t>>                  m_NodeToChildNodesTab;
    unordered_map<size_t, Actor::ActorId>   m_ActorIndexToIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

// static
IDag*    IDag::CreateDAG(const size_t total_nodes, const size_t root_nodes, const size_t rnd_bucket_size)
{
    assert(total_nodes > root_nodes);
    
    return new DAGImp(total_nodes, root_nodes, rnd_bucket_size);
}

} // namespace zamai

// nada mas



