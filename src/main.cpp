// main zama challenge

#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <signal.h>
#include <qb/actor.h>
#include <qb/main.h>

#include "zamai.h"

#include "lx/xutils.h"
#include "lx/xstring.h"

constexpr uint32_t  BLOWUP                      = 1;                    // try 1;

constexpr uint32_t  TOTAL_AVAIL_NODES            = 200 * BLOWUP;
constexpr uint32_t  ROOT_NODES                  = 4;                    // same as # of DAG "entry points", should be slightly smaller than physical # CPU cores
constexpr uint32_t  RANDOM_BUCKET_SIZE          = 5 * BLOWUP;

using namespace std;
using namespace qb;
using namespace zamai;
using namespace LX;

//---- Compute Event (sent to ComputeActors) -----------------------------------

struct ComputeEvent : Event
{
	ComputeEvent(const uint32_t v, const uint32_t n_computed = 0)
        : m_Val(v), m_NumComputed(n_computed)
    {
	}
    
	const uint32_t m_Val;               // payload
    const uint32_t m_NumComputed;
};

//---- Register Node event ----------------------------------------------------

struct RegisterNodeEvent : Event
{
    RegisterNodeEvent(const uint32_t id, const ActorId actor_id)
        : m_Id(id), m_ActorId(actor_id)
    {
    }
    
    const uint32_t          m_Id;
    const ActorId    m_ActorId;
};

//---- Path Termination event (so can countdown til end) -----------------------

struct PathTerminationEvent : Event  { };

//---- Service Init ------------------------------------------------------------

struct
ServiceInit
{
    // ctor
    ServiceInit(IDag *idag)
        : m_IDag(idag)
    {
        assert(idag);
    }
    
    IDag                        *m_IDag;
};

//----Registry Service ---------------------------------------------------------

struct Registry_serviceTag {};

class RegistryService: public ServiceActor<Registry_serviceTag>
{
public:

    // ctor
    RegistryService(const ServiceInit &init)
        : m_Dag(init.m_IDag),
        m_TotalTerminations(m_Dag->GetTotatlTerminations()), m_TerminatedCount(0),
        m_NumNodesRegistered(0)
    {
        registerEvent<RegisterNodeEvent>(*this);
        registerEvent<PathTerminationEvent>(*this);
    }
    
    // register node -> actor id, start event loops when all registered
    void    on(const RegisterNodeEvent &e)
    {
        m_Dag->RegisterActorId(e.m_Id, e.m_ActorId);
        
        m_NumNodesRegistered++;
        if (m_RegistrationLogStamp.elap_secs() > 2)
        {
            cout << xsprintf(" registered %d nodes\n", m_NumNodesRegistered);
            m_RegistrationLogStamp.reset();
        }
        
        // registered all nodes?
        if (m_NumNodesRegistered >= TOTAL_AVAIL_NODES)
        {   // send initial events to root nodes
        
            cout << endl << "all nodes registered -> starting async computations!" << endl << endl;
            
            m_StartStamp = timestamp_t();
            
            for (uint32_t i = 0; i < ROOT_NODES; i++)
            {
                const auto    aid = m_Dag->GetNodeActorId(i);

                push<ComputeEvent>(aid, 0);
            }
        }
    }
    
    // a DAG path has terminated (hit a leave)
    void    on(const PathTerminationEvent &e)
    {
        m_TerminatedCount++;
        
        if (m_TerminationLogStamp.elap_secs() >= 1)
        {
            cout << xsprintf(" PATH TERMINATION = %s / %s\n", LX::ToHumanBytes(m_TerminatedCount), LX::ToHumanBytes(m_TotalTerminations));
            
            m_TerminationLogStamp.reset();
        }
        
        if (m_TotalTerminations == m_TerminatedCount)
        {
            cout << "  elapsed = " << m_StartStamp.elap_str() << endl;
            
            // allow exit
            broadcast<KillEvent>();
        }
    }
    
private:

    IDag                        *m_Dag;
    const uint32_t              m_TotalTerminations;
    uint32_t                    m_TerminatedCount;
    uint32_t                    m_NumNodesRegistered;
    timestamp_t                 m_StartStamp, m_RegistrationLogStamp, m_TerminationLogStamp;
};

//---- Compute node initializer ------------------------------------------------

struct
ComputeInit
{
public:

    // ctor
    ComputeInit(IDag *dag, const uint32_t id, const uint32_t opmul, const uint32_t opbias)
        : m_IDag(dag), m_Id(id), m_OpMul(opmul), m_OpBias(opbias)
    {
        assert(dag);
    }
    
    IDag            *m_IDag;
    const uint32_t  m_Id;            // position in DAG vector/table
    const uint32_t  m_OpMul;    
    const uint32_t  m_OpBias;
};

//---- Compute Node actor ------------------------------------------------------

thread_local timestamp_t  t_LogStamp;

class ComputeActor
    : public Actor
{
public:

    // ctor
	ComputeActor(const ComputeInit &init)
        : m_Dag(init.m_IDag), m_Id(init.m_Id),
        m_OpMul(init.m_OpMul), m_OpBias(init.m_OpBias)
    {
		assert(m_Dag);
        
        t_LogStamp.reset();
        
		registerEvent<ComputeEvent>(*this);
        push<RegisterNodeEvent>(getServiceId<Registry_serviceTag>(0), m_Id, id());
    }
    
	// called when ComputeEvent is received
	void on(const ComputeEvent& e)
    {
		if (t_LogStamp.elap_secs() > 2)
        {   cout << "ComputeEvent: " << hex << e.m_Val << dec << " from " << e.getSource() << endl;
        
            t_LogStamp.reset();
        }
        
        // apply computation
        const uint32_t  rolling = (uint32_t) ((e.m_Val + m_OpBias) * m_OpMul);
        
        BroadcastToChildren(rolling, e.m_NumComputed + 1);
	}
    
private:

    // trickle-down to children
    void    BroadcastToChildren(const uint32_t val, const uint32_t n_computed)
    {
        const vector<uint32_t>    child_nodes = m_Dag->GetChildNodes(m_Id);
        assert(!child_nodes.empty());
        
        // send to child nodes
        for (const uint32_t child_id : child_nodes)
        {
            if (child_id == NODE_CHILD_END)
            {
                // cout << xsprintf(" END OF CHILD NODES\n");
                
                NotifyPathTermination();
                return;
            }
            
            const auto     child_actor_id = m_Dag->GetNodeActorId(child_id);
            
            push<ComputeEvent>(child_actor_id, val, n_computed);
        }
    }
    
    // send PathTerminationEvent to registry
    void    NotifyPathTermination(void)
    {
        push<PathTerminationEvent>(getServiceId<Registry_serviceTag>(0));
    }

    const IDag          *m_Dag;
    const uint32_t      m_Id;                   // index into DAG nodes
    const uint32_t      m_OpMul, m_OpBias;
};

//---- assert handler ----------------------------------------------------------

extern "C"
void my_assert_handler(int)
{
    //::kill(0, SIGTRAP);
}

//---- MAIN --------------------------------------------------------------------

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
	#ifndef NDEBUG
        signal(SIGABRT, &my_assert_handler);
    #endif

    cout << "zamai DAG w/ actor model *************************************************************" << endl << endl;
    
    cout << "  TOTAL_AVAIL_NODES = " << TOTAL_AVAIL_NODES << endl;
    cout << "  ROOT_NODES = " << ROOT_NODES << endl;
    cout << "  RANDOM_BUCKET_SIZE = " << RANDOM_BUCKET_SIZE << endl << endl;
    
    unique_ptr<IDag>            IDag(IDag::CreateDAG(TOTAL_AVAIL_NODES, ROOT_NODES, RANDOM_BUCKET_SIZE));
    
    qb::Main engine({0, 1, 2, 3});     // will use only cores [0; 3]
    
    engine.core(0).addActor<RegistryService>(ServiceInit(IDag.get()));
    
    auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
    
    cout << " instantiated nodes..." << endl;
     
    // some instantiated nodes may never be used (traversed) but that's ok as don't use CPU
    for (uint32_t i = 0; i < TOTAL_AVAIL_NODES; i++)
    {
        const uint32_t   cpu_core_id = i % ROOT_NODES;
        assert(cpu_core_id < ROOT_NODES);
        
        const uint32_t  op_mul = rnd_gen() * 0xffffu;
        const uint32_t  op_bias = rnd_gen() * 0xffffu;
        
        engine.addActor<ComputeActor>(cpu_core_id, ComputeInit(IDag.get(), i, op_mul, op_bias));
    }
    
    cout << " all nodes instantiated, starting CPU core event loop threads..." << endl;
    
    engine.start(false);
    engine.join();
    
    cout << "demo done!" << endl;
    
    return 0;
}

// nada mas
