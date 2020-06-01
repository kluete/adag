// main zama challenge

#include <iostream>
#include <vector>
#include <random>

#include "zamai.h"
#include "simplx.h"

constexpr size_t    TOTAL_NODES             = 50;
constexpr size_t    ROOT_NODES              = 4;                    // same as # of DAG "entry points", should be slightly smaller than # CPU cores
constexpr float     RANDOM_SLICE_FACTOR     = .1;                  // slice/chunk size, as factor of MAX_NODES

using namespace std;
using namespace tredzone;
using namespace zamai;

//---- Compute Event (sent to ComputeActors) -----------------------------------

struct ComputeEvent : Actor::Event
{
	ComputeEvent(const uint64_t &v)
        : m_Val(v)
    {
	}
    
	const uint64_t m_Val;               // payload
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

//---- Service Init ------------------------------------------------------------

class
ServiceInit
{
public:
    // ctor
    ServiceInit(IDag *idag)
        : m_IDag(idag)
    {
        assert(idag);
        
        
    }
    
    IDag    *m_IDag;
};

//----Registry Service ---------------------------------------------------------

struct Registry_serviceTag: public Service {};

class RegistryService: public Actor
{
public:

    // ctor
    RegistryService(const ServiceInit &init)
        : m_Dag(init.m_IDag)
    {
        registerEventHandler<RegisterNodeEvent>(*this);
    }
    
    void    onEvent(const RegisterNodeEvent &e)
    {
        cout << "RegisterNodeEvent(i = " << e.m_Index << ")" << endl;
        
        m_Dag->RegisterIndexActorId(e.m_Index, e.m_ActorId);
    }
    
private:

    IDag    *m_Dag;
};

//---- Compute node initializer ------------------------------------------------

class
ComputeInit
{
public:

    // ctor
    ComputeInit(IDag *dag, const size_t index, const uint32_t &opmul, const uint32_t &opbias)
        : m_IDag(dag), m_Index(index), m_OpMul(opmul), m_OpBias(opbias)
    {
        assert(dag);
    }
    
private:

    IDag            *m_IDag;
    const size_t    m_Index;            // position in DAG tab
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
        const uint32_t  rolling = (uint32_t) ((e.m_Val * m_OpMul) + m_OpBias);
        
        BroadcastToChildren(rolling);
	}
    
    void onCallback()
    {
		if (!m_Dag->IsIndexRegistered(m_Index))
        {   // can register now that has true core position (should happen once), but not yet start events
            cout << "registering actor ID " << m_Index << endl;
    
            const ActorId    aid = getActorId();
        
            m_Dag->RegisterIndexActorId(m_Index, aid);
        }
        
        if (m_Index < (ROOT_NODES -1))       return;         // not a root node
        
        // if all nodes are registered
        if (m_Dag->GetNumRegisteredIndices() != TOTAL_NODES)
        {
            // not yet fully registered -> reload callback
            registerCallback(*this);
        }
        else
        {   // fully registered, start events
            cout << "actor " << m_Index << " is starting events" << endl;
                
            // now that all actors are registered, start branch computation
            BroadcastToChildren(0/*init value*/);
        }
	}
    
private:

    // trickle-down to children
    void    BroadcastToChildren(const uint32_t val)
    {
        const vector<size_t>    child_nodes = m_Dag->GetChildNodes(m_Index);
        if (child_nodes.empty())
        {
            cout << " NO MORE CHILD NODES" << endl;
            return;
        }
        
        // send to child nodes
        for (const size_t child_id: child_nodes)
        {
            if (child_id == 0)
            {
                cout << " END OF CHILD NODES" << endl;   
                return;
            }
            
            const Actor::ActorId     child_actor_id = m_Dag->GetNodeActorId(child_id);
            
            Event::Pipe pipe_to_child(*this, child_actor_id);
            
            pipe_to_child.push<ComputeEvent>(val);
        }
    }

    IDag            *m_Dag;
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

    cout << "zamai DAG w/ actor model" << endl;
    
    unique_ptr<IDag>   IDag(IDag::CreateDAG(TOTAL_NODES, ROOT_NODES, RANDOM_SLICE_FACTOR));
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    startSequence.addServiceActor<Registry_serviceTag, RegistryService>(0, ServiceInit(IDag.get()));
    
    auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
    
    // some instantiated nodes may never be used (traversed) but that's ok as don't use CPU
    for (size_t i = 0; i < TOTAL_NODES; i++)
    {
        const size_t   cpu_core_id = i % ROOT_NODES;
        assert(cpu_core_id < ROOT_NODES);
        
        const uint32_t  op_mul = rnd_gen() * 0xffffu;
        const uint32_t  op_bias = rnd_gen() * 0xffffu;
        
        startSequence.addActor<ComputeActor>(cpu_core_id, ComputeInit(IDag.get(), i, op_mul, op_bias));
    }
    
    Engine engine(startSequence);	                // start above actors

    cout << "Press enter to exit...";
    cin.get();

    return 0;
}

// nada mas
