// main zama challenge

#include <iostream>
#include <vector>
#include <random>

#include "simplx.h"

#include "zamai.h"
#include "trz/util/waitcondition.h"

#include "lx/xutils.h"
#include "lx/xstring.h"

constexpr size_t    TOTAL_NODES             = 100'000;
constexpr size_t    ROOT_NODES              = 4;                    // same as # of DAG "entry points", should be slightly smaller than # CPU cores
constexpr float     RANDOM_BUCKET_FACTOR    = .001;                 // slice/chunk size, as factor of MAX_NODES
constexpr size_t    NODE_REGISTRATION_BATCH = 1000;                 // how often to log to cout

using namespace std;
using namespace tredzone;
using namespace zamai;
using namespace LX;

//---- Compute Event (sent to ComputeActors) -----------------------------------

struct ComputeEvent : Actor::Event
{
	ComputeEvent(const uint32_t &v)
        : m_Val(v)
    {
	}
    
	const uint32_t m_Val;               // payload
};

//---- Register Node event ----------------------------------------------------

struct RegisterNodeEvent : Actor::Event
{
    RegisterNodeEvent(const size_t index, const Actor::ActorId actor_id)
        : m_Index(index), m_ActorId(actor_id)
    {
    }
    
    const size_t            m_Index;
    const Actor::ActorId    m_ActorId;
};

//---- Path Termination event (so can countdown til end) -----------------------

struct PathTerminationEvent : Actor::Event  { };

//---- Service Init ------------------------------------------------------------

struct
ServiceInit
{
    // ctor
    ServiceInit(IDag *idag, shared_ptr<IWaitCondition> wait_cond)
        : m_IDag(idag), m_IWaitCond(wait_cond)
    {
        assert(idag);
    }
    
    IDag                        *m_IDag;
    shared_ptr<IWaitCondition>  m_IWaitCond;
};

//----Registry Service ---------------------------------------------------------

struct Registry_serviceTag: public Service {};

class RegistryService: public Actor
{
public:

    // ctor
    RegistryService(const ServiceInit &init)
        : m_Dag(init.m_IDag), m_WaitCondition(init.m_IWaitCond),
        m_TotalTerminations(m_Dag->GetTotatlTerminations()), m_TerminatedCount(0),
        m_NumNodesRegistered(0)
    {
        registerEventHandler<RegisterNodeEvent>(*this);
        registerEventHandler<PathTerminationEvent>(*this);
    }
    
    // register node -> actor id, start event loops when all registered
    void    onEvent(const RegisterNodeEvent &e)
    {
        m_Dag->RegisterIndexActorId(e.m_Index, e.m_ActorId);
        
        m_NumNodesRegistered++;
        if (m_NumNodesRegistered % NODE_REGISTRATION_BATCH == 0)
        {
            cout << " registered " << m_NumNodesRegistered << " nodes" << endl;
        }
        
        // registered all nodes?
        if (m_NumNodesRegistered >= TOTAL_NODES)
        {   // send initial events to root nodes
        
            cout << endl << "all nodes registered -> starting async computations!" << endl << endl;
            
            m_StartStamp = timestamp_t();
            
            for (size_t i = 0; i < ROOT_NODES; i++)
            {
                const Actor::ActorId    aid = m_Dag->GetNodeActorId(i);
                
                Event::Pipe pipe_to_child(*this, aid);
            
                pipe_to_child.push<ComputeEvent>(0/*init val*/);
            }
        }
    }
    
    // a DAG path has terminated (hit a leave)
    void    onEvent(const PathTerminationEvent &e)
    {
        m_TerminatedCount++;
        
        cout << " PATH TERMINATION = " << m_TerminatedCount << " / " << m_TotalTerminations << endl;
        
        if (m_TotalTerminations == m_TerminatedCount)
        {
            cout << "  elapsed = " << m_StartStamp.elap_str() << endl;
            
            // allow exit
            m_WaitCondition->notify();
        }
    }
    
private:

    IDag                        *m_Dag;
    shared_ptr<IWaitCondition>  m_WaitCondition;
    const size_t                m_TotalTerminations;
    size_t                      m_TerminatedCount;
    size_t                      m_NumNodesRegistered;
    timestamp_t                 m_StartStamp;
};

//---- Compute node initializer ------------------------------------------------

struct
ComputeInit
{
public:

    // ctor
    ComputeInit(IDag *dag, const size_t index, const uint32_t &opmul, const uint32_t &opbias)
        : m_IDag(dag), m_Index(index), m_OpMul(opmul), m_OpBias(opbias)
    {
        assert(dag);
    }
    
    IDag            *m_IDag;
    const size_t    m_Index;            // position in DAG vector/table
    const uint32_t  m_OpMul;    
    const uint32_t  m_OpBias;
};

//---- Compute Node actor ------------------------------------------------------

class ComputeActor : public Actor, public Actor::Callback
{
public:

    // ctor
	ComputeActor(const ComputeInit &init)
        : m_Dag(init.m_IDag), m_Index(init.m_Index),
        m_OpMul(init.m_OpMul), m_OpBias(init.m_OpBias)
    {
		// cout << "ComputeActor::ComputeActor()" << endl;
        
        assert(m_Dag);
        
        registerCallback(*this);	                // callback once is instantiated on right cpu/core
		registerEventHandler<ComputeEvent>(*this);
    }
    
	// called when ComputeEvent is received
	void onEvent(const ComputeEvent& e)
    {
		cout << "ComputeActor::onEvent(): " << hex << e.m_Val << dec << " from " << e.getSourceActorId() << endl;
        
        // apply computation
        const uint32_t  rolling = (uint32_t) ((e.m_Val + m_OpBias) * m_OpMul);
        
        BroadcastToChildren(rolling);
	}
    
    void onCallback()
    {
		// can register now that has true core position (should happen once), but not yet start events
        const ActorId	RegistryActorId = getEngine().getServiceIndex().getServiceActorId<Registry_serviceTag>();
        
        Event::Pipe pipe_to_registry(*this, RegistryActorId);
            
        pipe_to_registry.push<RegisterNodeEvent>(m_Index, getActorId());
	}
    
private:

    // trickle-down to children
    void    BroadcastToChildren(const uint32_t val)
    {
        const vector<size_t>    child_nodes = m_Dag->GetChildNodes(m_Index);
        if (child_nodes.empty())
        {
            // cout << " NO MORE CHILD NODES" << endl;
            NotifyPathTermination();
            return;
        }
        
        // send to child nodes
        for (const size_t child_id: child_nodes)
        {
            if (child_id == 0)
            {
                // cout << " END OF CHILD NODES" << endl;
                NotifyPathTermination();
                return;
            }
            
            const Actor::ActorId     child_actor_id = m_Dag->GetNodeActorId(child_id);
            
            Event::Pipe pipe_to_child(*this, child_actor_id);
            
            pipe_to_child.push<ComputeEvent>(val);
        }
    }
    
    // send PathTerminationEvent to registry
    void    NotifyPathTermination(void)
    {
        const ActorId	RegistryActorId = getEngine().getServiceIndex().getServiceActorId<Registry_serviceTag>();
        
        Event::Pipe pipe_to_registry(*this, RegistryActorId);
            
        pipe_to_registry.push<PathTerminationEvent>();
    }

    const IDag      *m_Dag;
    const size_t    m_Index;
    const uint32_t  m_OpMul, m_OpBias;
};

//---- assert handler ----------------------------------------------------------

extern "C"
void my_assert_handler(int)
{
    ::kill(0, SIGTRAP);
}

//---- MAIN --------------------------------------------------------------------

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
	#ifndef NDEBUG
        signal(SIGABRT, &my_assert_handler);
    #endif

    cout << "zamai DAG w/ actor model *************************************************************" << endl;
    
    unique_ptr<IDag>            IDag(IDag::CreateDAG(TOTAL_NODES, ROOT_NODES, RANDOM_BUCKET_FACTOR));
    shared_ptr<IWaitCondition>  wait_condition(IWaitCondition::Create());
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    startSequence.addServiceActor<Registry_serviceTag, RegistryService>(0, ServiceInit(IDag.get(), wait_condition));
    
    auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
    
    cout << " instantiated nodes..." << endl;
     
    // some instantiated nodes may never be used (traversed) but that's ok as don't use CPU
    for (size_t i = 0; i < TOTAL_NODES; i++)
    {
        const size_t   cpu_core_id = i % ROOT_NODES;
        assert(cpu_core_id < ROOT_NODES);
        
        const uint32_t  op_mul = rnd_gen() * 0xffffu;
        const uint32_t  op_bias = rnd_gen() * 0xffffu;
        
        startSequence.addActor<ComputeActor>(cpu_core_id, ComputeInit(IDag.get(), i, op_mul, op_bias));
    }
    
    cout << " all nodes instantiated, starting CPU core event loop threads..." << endl;
    
    Engine engine(startSequence);	                // start above actors

    wait_condition->wait();
    
    cout << "demo done!" << endl;
    
    return 0;
}

// nada mas
