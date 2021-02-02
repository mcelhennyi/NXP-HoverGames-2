#include <iostream>

#define BASE_MODE 1 // 1 enables base mode, else this will run as a agent.

#include <unistd.h>
#include <signal.h>
#include <cstdlib>

#if BASE_MODE
#include <system/base_station/BaseStation.h>
#else
#include <system/agent/Agent.h>
#endif

using namespace std;

bool run = true;

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void keyboardInterruptCallback(int signum)
{
    // Terminate program
    run = false;
}

int main()
{
    // Setup signal catcher
    signal(SIGINT, keyboardInterruptCallback);
    // TODO add more for exceptions?

    // Setup our system code
#if BASE_MODE
    // Base station
    std::cout << "Running base station." << std::endl;
    auto system = new ::System::BaseStation();
#else
    // Agent station
    std::cout << "Running agent." << std::endl;
    auto system = new ::System::Agent();
#endif

    // Setup the system
    system->setup();

    // Run the system
    system->run();

    // Hold, if we ctrl-c cleanup and end
    while(run)
    {
        // Allows for an exit every second (so 1 second to react to a ctrl-c action)
        usleep(1000000 * 1);
    }

    // Clean up the system
    system->stop();

    return EXIT_SUCCESS;
}
