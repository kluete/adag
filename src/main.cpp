// main zama challenge

#include <iostream>
#include <vector>
#include <random>

#include "zamai.h"
#include "simplx.h"

constexpr size_t    NUM_TOTAL_NODES         = 1000;
constexpr size_t    NUM_ROOT_NODES          = 4;                    // same as # of DAG "entry points", should be slightly smaller than # CPU cores
constexpr float     RANDOM_SLICE_FACTOR     = .05;                  // slice/chunk size, as factor of MAX_NODES

using namespace std;
using namespace tredzone;
using namespace zamai;

//---- Compute Event -----------------------------------------------------------

// Event sent between ComputeActors, payload is int32 value

struct ComputeEvent : Actor::Event
{
	ComputeEvent(const uint32_t &v)
        : m_Val(v)
    {
	}
    
	const uint32_t m_Val;               // payload
};

//---- Compute node initializer ------------------------------------------------

class
ComputeInit
{
public:

    // ctor
    ComputeInit(IDag *dag, const size_t index, const uint32_t &opmul)
        : m_IDag(dag), m_Index(index), m_OpMul(opmul)
    {
        assert(dag);
    }
    
    void    RegisterActorId(void)
    {
        
    }
    
private:

    IDag            *m_IDag;
    const size_t    m_Index;
    const uint32_t  m_OpMul;    
};

//---- Compute Node actor ------------------------------------------------------

class ComputeActor : public Actor, public Actor::Callback
{
public:

    // ctor
	ComputeActor(const ComputeInit &init)
        : m_Dag(init.m_IDag), m_Index(init.m_Index), m_OpMul(init.m_OpMul)
    {
		cout << "ComputeActor::ComputeActor()" << endl;
        
        assert(m_Dag);
        
        registerCallback(*this);	                // callback once is instantiated on right cpu/core
		registerEventHandler<ComputeEvent>(*this);
    }
    
	// called when ComputeEvent is received
	void onEvent(const ComputeEvent& e)
    {
		cout << "ComputeActor::onEvent(): " << e.m_Val << ", from " << e.getSourceActorId() << endl;
	}
    
    void onCallback()
    {
		cout << "registering actor ID " << m_Index << endl;
        
        // can register now that has true core position
        const ActorId    aid = getActorId();
        
        m_Dag->RegisterIndexActorId(m_Index, aid);
	}
    
private:

    IDag            *m_Dag;
    const size_t    m_Index;
    const uint32_t  m_OpMul;
};

//---- Main --------------------------------------------------------------------

int main()
{
    cout << "zamai DAG w/ actor model" << endl;
    
    unique_ptr<IDag>   IDag(IDag::CreateDAG(NUM_TOTAL_NODES, NUM_ROOT_NODES, RANDOM_SLICE_FACTOR));
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    auto	rnd_gen = bind(uniform_real_distribution<>(0, 1.0), default_random_engine{0/*seed*/});
    
    for (size_t i = 0; i < NUM_TOTAL_NODES; i++)
    {
        const size_t   cpu_core_id = i % NUM_ROOT_NODES;
        assert(cpu_core_id < NUM_ROOT_NODES);
        
        const uint32_t  op_mul = rnd_gen() * 0xffffu;
        
        startSequence.addActor<ComputeActor>(cpu_core_id, ComputeInit(IDag.get(), i, op_mul));
    }
    
    Engine engine(startSequence);	                // start above actors

    cout << "Press enter to exit...";
    cin.get();

    return 0;
}

// nada mas
