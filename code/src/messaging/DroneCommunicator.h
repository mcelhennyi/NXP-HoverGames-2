//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_DRONECOMMUNICATOR_H
#define HOVERGAMES2_DRONECOMMUNICATOR_H

#include "Communicator.h"

namespace Messaging
{
    class DroneCommunicator: public Communicator
    {
    public:
        DroneCommunicator();
        ~DroneCommunicator() {};

    public:
        void sendHello();
        void sendAgentLocation(Location &currentLocation, Location &targetLocation);

    protected:
        CommDetails _baseStation;

    };
}

#endif //HOVERGAMES2_DRONECOMMUNICATOR_H
