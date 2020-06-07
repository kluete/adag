// DAG implementation

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <random>
#include <thread>
#include <mutex>
#include <limits>
#include <sstream>
#include <algorithm>            // for std::max

#include "simplx.h"

#include "zamai.h"

#include "lx/xutils.h"
#include "lx/xstring.h"

namespace zamai
{
using namespace std;
using namespace tredzone;
using namespace LX;

//---- DAG imp -----------------------------------------------------------------

class DAGImp : public IDag
{
public:
    // ctor
    DAGImp(const uint32_t total_nodes, const uint32_t root_nodes, const uint32_t rnd_bucket_size)
        : m_TotalNodes(total_nodes), m_RootNodes(root_nodes), m_RndBucketSize(rnd_bucket_size),
        m_MaxBranchNodes((m_TotalNodes - m_RootNodes) / m_RndBucketSize),
        m_TraversedNodes(0), m_MaxTraversedDepth(0), m_MaxNodeId(0), m_TotalTerminations(0)
    {
        assert(m_MaxBranchNodes > 0);
        
        cout << "DAGImp() CTOR" << endl;
        cout << "  m_MaxBranchNodes = " << m_MaxBranchNodes << endl;
        
        CreateDAG();
        
        // calc total path terminations
        m_TotalTerminations = CalcTotalTerminations();
    }
    
    uint32_t    GetTotatlTerminations(void) const override
    {
        return m_TotalTerminations;
    }
    
    void    RegisterActorId(const uint32_t id, const Actor::ActorId &actor_id) override
    {
        assert(id < m_TotalNodes);
        
        // check wasn't already registered
        assert(!m_NodeToActorIdMap.count(id));
        
        // m_NodeToActorIdMap.insert({id, actor_id});
        m_NodeToActorIdMap.insert({id, actor_id});
    }
   
    vector<uint32_t>  GetChildNodes(const uint32_t id) const override
    {
        assert(id < m_TotalNodes);
        
        // mode always exists (though may be empty)
        
        const vector<uint32_t>    children = m_NodeToChildNodesTab.at(id);
        
        return children;
    }
    
    Actor::ActorId  GetNodeActorId(const uint32_t id) const override
    {
        assert(id < m_TotalNodes);
        
        // make sure exists (though may be empty actor ID)
        assert(m_NodeToActorIdMap.count(id));
        
        const Actor::ActorId    aid = m_NodeToActorIdMap.at(id);
        // check isn't null actor id
        assert(aid != Actor::ActorId());
        
        return aid;
    }

private:
    
    inline
    uint64_t    make_edge(const uint32_t n1, const uint32_t n2) const
    {
        const uint64_t    key(((uint64_t)n1) << 32 | n2);
        return key;
    }
    
    // generate index -> index graph
    void CreateDAG(void)
    {
        cout << "building DAG..." << endl << endl;
        
        m_NodeToChildNodesTab.resize(m_TotalNodes, {}/*empty child list*/);
        
        assert(m_RndBucketSize > 1);
        
        auto	rnd_gen = bind(uniform_int_distribution<>(1, m_RndBucketSize - 1), default_random_engine{0/*seed*/});
        
        uint32_t  max_node_children = 0;
        
        // per branch (1 branch = 1 thread)
        for (uint32_t root_node = 0; root_node < m_RootNodes; root_node++)
        {
            uint32_t  walker_node = root_node;
            
            cout << "  th[" << root_node << "]:" << endl;
            
            // generate child nodes
            for (uint32_t j = 0; j < m_MaxBranchNodes; j++)
            {
                const uint32_t  next_node = ((j + 1) * m_RndBucketSize) + rnd_gen();       // prevent root nodes to burrow into one another
                assert(next_node > walker_node);
                
                assert(walker_node < m_TotalNodes);
                assert(next_node < m_TotalNodes);
                assert(walker_node < next_node);       // don't loop over self or upstream
                
                m_NodeToChildNodesTab[walker_node].push_back(next_node);
                
                // compute any node's manimum # of children
                max_node_children = std::max(max_node_children, (uint32_t) m_NodeToChildNodesTab[walker_node].size());
                
                // cout << "     node[" << j << "] : " << walker_node << " -> " << next_node << endl;
                
                walker_node = next_node;
            }
            
            // end branch
            m_NodeToChildNodesTab[walker_node].push_back(NODE_CHILD_END);
            
             cout << endl << "DAG done, max_node_children = " << max_node_children << endl << endl;
        }
        
        // sort DAG unique children
        for (auto &children : m_NodeToChildNodesTab)
        {
            sort(children.begin(), children.end());
            children.erase(unique( children.begin(), children.end() ), children.end());
        }
    }
    
    void    DumpDAG(void) const override
    {
        for (size_t i = 0; i < m_NodeToChildNodesTab.size(); i++)
        {
            const vector<uint32_t> &child_list = m_NodeToChildNodesTab[i];
            
            if (child_list.empty())     continue;
            
            ostringstream   oss;
            
            oss << xsprintf("%4d : ", i);
            
            for (const uint32_t child : child_list)
            {
                if (child != NODE_CHILD_END)
                        oss << child << " ";
                else    oss << "NODE_CHILD_END";
            }
            
            cout << oss.str() << endl;
        }
    }
    
    // calc path terminations (RECURSIVE)
    void    CalcPathTerminations(const uint32_t root_node, const uint32_t parent_node, uint32_t &n_path_nodes, const uint32_t depth) const
    {
        m_TraversedNodes++;
        
        m_MaxTraversedDepth = std::max(m_MaxTraversedDepth, depth);
        
        const vector<uint32_t>    child_nodes = GetChildNodes(parent_node);
        
        assert(!child_nodes.empty());
        
        m_MaxChildren = std::max(m_MaxChildren, (uint32_t)child_nodes.size());
        
        // send to child nodes
        // for (const uint32_t child_id: child_nodes)
        for (size_t i = 0; i < child_nodes.size(); i++)
        {
            const string    pos_s(xsprintf("%d.%d", parent_node, i));
            
            const uint32_t child_id = child_nodes[i];
            
            if (child_id == NODE_CHILD_END)
            {
                // end node marker
                n_path_nodes++;
                
                cout << " NODE_CHILD_END " << endl;
                return;                                     // is that enough?
            }
            
            assert(child_id > parent_node);
            
            m_MaxNodeId = std::max(m_MaxNodeId, child_id);
            
            /*
            // can't prevent multiple edge traversal with fanning & merging
            const uint64_t  edge = make_edge(child_id, parent_node);
  
            const size_t    n_visited_edges = m_VisitedEdgeMap.count(edge) ? m_VisitedEdgeMap.at(edge) + 1 : 1;
            m_VisitedEdgeMap.emplace(edge, n_visited_edges);
            */
            
            //if (0 == m_TraversedNodes % TERMINATION_LOG_BATCH)
            {
                // cout << " node traversal: root = " << root_node << ", n_traversed = " << m_TraversedNodes << ", parent_node = " << parent_node << ", this_node = " << child_id << ", max_id = " << m_MaxNodeId << ", depth = " << depth << "/" << m_MaxTraversedDepth << ", n_visited_edges = " << n_visited_edges << endl;
                cout << "pos = " << pos_s << ", this_node = " << child_id << ", max_node = " << m_MaxNodeId << ", depth = " << depth << endl;
            }
            
            /*
            assert(!m_VisitedPosSet.count(pos_s));
            m_VisitedPosSet.emplace(pos_s);
            */
            
            // RECURSE
            CalcPathTerminations(root_node, child_id, n_path_nodes, depth + 1);
        }
    }
    
    uint32_t  CalcTotalTerminations(void) const
    {
        cout << "calculating total # of terminations..." << endl;
        
        uint32_t  n_endings  = 0;
        
        for (uint32_t root_node = 0; root_node < m_RootNodes; root_node++)
        {
            m_VisitedEdgeMap.clear();
            m_MaxNodeId = 0;
            m_MaxChildren = 0;
            
            uint32_t  n_path_nodes = 0;
            
            CalcPathTerminations(root_node, root_node, n_path_nodes/*&*/, 0/*depth*/);
            
            n_endings += n_path_nodes;
        }

        cout << "  DAG has " << n_endings << " total # of terminations..." << endl;
        
        return n_endings;
    }

    const uint32_t    m_TotalNodes;
    const uint32_t    m_RootNodes;
    const uint32_t    m_RndBucketSize;
    const uint32_t    m_MaxBranchNodes;
    
    mutable uint32_t  m_TraversedNodes;
    mutable uint32_t  m_MaxTraversedDepth;
    mutable uint32_t  m_MaxNodeId;
    mutable uint32_t  m_MaxChildren;
    uint32_t          m_TotalTerminations;
    
    // mutable unordered_set<uint64_t>  m_VisitedEdgeSet;
    mutable unordered_map<uint64_t, size_t>  m_VisitedEdgeMap;
    mutable unordered_set<string>           m_VisitedPosSet;
    
    vector<vector<uint32_t>>                  m_NodeToChildNodesTab;
    unordered_map<uint32_t, Actor::ActorId>   m_NodeToActorIdMap;
};

//---- INSTANTIATION -----------------------------------------------------------

// static
IDag*    IDag::CreateDAG(const uint32_t total_nodes, const uint32_t root_nodes, const uint32_t rnd_bucket_size)
{
    assert(total_nodes > root_nodes);
    
    return new DAGImp(total_nodes, root_nodes, rnd_bucket_size);
}

} // namespace zamai

// nada mas



