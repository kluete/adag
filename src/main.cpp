// main zama challenge
#include <iostream>

#include "simplx.h"

using namespace std;
using namespace tredzone;

// Event be sent between ComputeActors, payload is int32 value

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

//---- Writer Actor ------------------------------------------------------------

    // sends a PrintEvent containing a string to PrinterActor
    
class WriterActor : public Actor
{
public:
    // ctor
    WriterActor()
    {
		cout << "WriterActor::CTOR()" << endl;
        
        // retrieve PrinterActor's id from the ServiceIndex
		const ActorId&   printerActorId = getEngine().getServiceIndex().getServiceActorId<ComputeActor::ServiceTag>();
        
		Event::Pipe pipe(*this, printerActorId);	// create uni-directional communication channel between WriterActor (this) and PrinterActor (printerActorId)
		pipe.push<ComputeEvent>("Hello, World!");		// send PrintEvent through pipe
	}
};

//---- Main --------------------------------------------------------------------

int main()
{
    cout << "zamai DAG actor model" << endl;
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    startSequence.addActor<ComputeActor>(0/*CPU core*/);
    startSequence.addActor<ComputeActor>(0/*CPU core*/);

    Engine engine(startSequence);	                // start above actors

    cout << "Press enter to exit...";
    cin.get();

    return 0;
}

// nada mas
