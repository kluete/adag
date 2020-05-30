// main zama challenge
#include <iostream>

#include "simplx.h"

using namespace std;
using namespace tredzone;

int main()
{
    cout << "tutorial #4 : printer actor service" << endl;
    
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
