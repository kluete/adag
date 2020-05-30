// main zama challenge
#include <iostream>

#include "zamai.h"
#include "simplx.h"

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

//---- Compute Actor -----------------------------------------------------------

class ComputeActor : public Actor
{
public:

    // ctor
	ComputeActor()
    {
		cout << "ComputeActor::ComputeActor()" << endl;
		registerEventHandler<ComputeEvent>(*this);
    }
    
	// called when PrintEvent is received
	void onEvent(const ComputeEvent& e)
    {
		cout << "ComputeActor::onEvent(): " << e.m_Val << ", from " << e.getSourceActorId() << endl;
	}
};

//---- Main --------------------------------------------------------------------

int main()
{
    cout << "zamai DAG w/ actor model" << endl;
    
    unique_ptr<IDag>   IDag(IDag::CreateDAG());
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    startSequence.addActor<ComputeActor>(0/*CPU core*/);
    startSequence.addActor<ComputeActor>(0/*CPU core*/);

    Engine engine(startSequence);	                // start above actors

    cout << "Press enter to exit...";
    cin.get();

    return 0;
}

// nada mas
