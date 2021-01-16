//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_BASESTATIONCOMMUNICATOR_H
#define HOVERGAMES2_BASESTATIONCOMMUNICATOR_H

#include "../utils/util_time.h"

#include "Communicator.h"

#include <map>

namespace Messaging
{
    class BaseStationCommunicator: public Communicator
    {
    public:
        BaseStationCommunicator(char id): Communicator(id) {};
        ~BaseStationCommunicator() {};

    public:
        void sendWelcome(std::string ipAddr, int port, char nodeId);
        void forwardAgentLocation(char targetId, AgentLocation *agentLocationMessage);
        void forwardSubjectLocation(char targetId, SubjectLocation *subjectLocationMessage);

        void sendAgentMove(char targetId, Location targetLoaction);

    };
}

#endif //HOVERGAMES2_BASESTATIONCOMMUNICATOR_H
