//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_BASESTATION_H
#define HOVERGAMES2_BASESTATION_H

#include <vector>
#include "../../messaging/BaseStationCommunicator.h"

#define NODE_ID_START 5

namespace System
{
    class BaseStation
    {
    public:

        BaseStation() {};

        ~BaseStation() {};

        void setup();
        void run();

    private:

        // Network setup
        void handleNewNode(char *helloMessage);

        // Generic handlers
        void handleAck(char *ackMessage);

        // Agent methods
        void handleAgentLocation(char *agentLocationMessage);

        // Controller methods
        void handleSubjectLocation(char *subjectLocationMessage);
        void handleAgentMoveCommand(char *agentMoveCommandMessage);

        bool isAgent(char nodeId);
        bool isController(char nodeId);

    private:
        bool _running;
        int _nodeCount;

        // Communicator to talk out of this process
        Messaging::BaseStationCommunicator *_communicator;

        // Keep track of acks
        std::vector<char> _activeAgents;
        std::vector<char> _activeControllers;

    };
}

#endif //HOVERGAMES2_BASESTATION_H
