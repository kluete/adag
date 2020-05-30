// main zama challenge
#include <iostream>

#include "simplx.h"

using namespace std;
using namespace tredzone;

/*
 * Event that will be sent from the WriterActor to the PrinterActor.
 * The payload is a string that will be printed to the console.
 */
struct PrintEvent : Actor::Event
{
	PrintEvent(const string& message)
        : message(message)
    {
	}
    
	const string message;
};

//---- PrinterActor ------------------------------------------------------------

    // when receives PrintEvents it displays them to the console
    
class PrinterActor : public Actor
{
public:

	// service tag (or key) used to uniquely identify service
	struct ServiceTag : Service {};

    // ctor
	PrinterActor()
    {
		cout << "PrinterActor::PrinterActor()" << endl;
		registerEventHandler<PrintEvent>(*this);
    }
    
	// called when PrintEvent is received
	void onEvent(const PrintEvent& event)
    {
		cout << "PrinterActor::onEvent(): " << event.message << ", from " << event.getSourceActorId() << endl;
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
		const ActorId&   printerActorId = getEngine().getServiceIndex().getServiceActorId<PrinterActor::ServiceTag>();
        
		Event::Pipe pipe(*this, printerActorId);	// create uni-directional communication channel between WriterActor (this) and PrinterActor (printerActorId)
		pipe.push<PrintEvent>("Hello, World!");		// send PrintEvent through pipe
	}
};

//---- Main --------------------------------------------------------------------

int main()
{
    cout << "zamai DAG actor model" << endl;
    
    Engine::StartSequence   startSequence;	        // configure initial Actor system
    
    startSequence.addServiceActor<PrinterActor::ServiceTag, PrinterActor>(0/*CPU core*/);
    startSequence.addActor<WriterActor>(0/*CPU core*/);
    startSequence.addActor<WriterActor>(0/*CPU core*/);

    Engine engine(startSequence);	                // start above actors

    cout << "Press enter to exit...";
    cin.get();

    return 0;
}

// nada mas
