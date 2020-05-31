// main zama challenge
#include <iostream>
#include <vector>
#include <random>

#include "zamai.h"
#include "simplx.h"

constexpr size_t    MAX_NODES               = 1000;
constexpr size_t    MAX_NONE_FANNING        = 10;
constexpr int       NUM_THREADS             = 4;        // should be slightly smaller than # CPU cores, equivalent to # of DAG "entry points"

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
    
	const uint32_t m_Val;
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
    
	// called when PrintEvent is received
	void onEvent(const ComputeEvent& e)
    {
		cout << "ComputeActor::onEvent(): " << e.m_Val << ", from " << e.getSourceActorId() << endl;
	}
    
    void onCallback()
    {
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
    
    unique_ptr<IDag>   IDag(IDag::CreateDAG(MAX_NODES, MAX_NONE_FANNING, NUM_THREADS));
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    startSequence.addActor<ComputeActor>(0/*CPU core*/, ComputeInit(IDag.get(), 0, 0x12));
    startSequence.addActor<ComputeActor>(0/*CPU core*/, ComputeInit(IDag.get(), 1, 0x34));

    Engine engine(startSequence);	                // start above actors

    cout << "Press enter to exit...";
    cin.get();

    return 0;
}

// nada mas
