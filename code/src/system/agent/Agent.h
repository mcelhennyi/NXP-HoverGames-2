//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_AGENT_H
#define HOVERGAMES2_AGENT_H

#include "../Runnable.h"
#include "../../messaging/DroneCommunicator.h"

//#include <chrono>
//#include <cstdint>
//#include <mavsdk/mavsdk.h>
//#include <mavsdk/plugins/action/action.h>
//#include <mavsdk/plugins/telemetry/telemetry.h>
//#include <iostream>
//#include <thread>
//
//using namespace mavsdk;
//using namespace std::this_thread;
//using namespace std::chrono;


namespace System
{
    class Agent: public Runnable
    {
    public:
        Agent();
        ~Agent();

        void doSetup() override;
        void doRun() override;
        void doStop() override;


    private:
        // Communicator to talk out of this process
        Messaging::DroneCommunicator      *_communicator;

    };
}

#endif //HOVERGAMES2_AGENT_H
