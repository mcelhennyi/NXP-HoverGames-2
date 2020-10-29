//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_BASESTATION_H
#define HOVERGAMES2_BASESTATION_H

#include <vector>

#include "../../messaging/BaseStationCommunicator.h"

#include "../Runnable.h"

#define BASE_ID 4
#define NODE_ID_START 5

namespace System
{
    class BaseStation: public Runnable
    {
    public:

        BaseStation();

        ~BaseStation();

        void doSetup() override;
        void doRun() override;
        void doStop() override;

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

        struct ControllerParams
        {
            Location        controllerToBaseOffset;
        };

        struct AgentParams
        {
            unsigned long   lastLocationUpdateTime;
            Location        agentLocation;

            // Origin location
            bool            originSet;
            Location        agentOriginOffset;
        };

        bool                                    _running;
        int                                     _nodeCount;

        // Communicator to talk out of this process
        Messaging::BaseStationCommunicator      *_communicator;

        // Track the agents and controllers
        std::map<char, AgentParams>             _activeAgents;
        std::map<char, ControllerParams>        _activeControllers;

        std::map<char, char>                    _agentOwnerMap; // Agent (first) <-- Owner (second)


    };
}

#endif //HOVERGAMES2_BASESTATION_H
