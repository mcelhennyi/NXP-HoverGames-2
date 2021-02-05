//
// Created by user on 10/17/20.
//

#include <iostream>
#include <unistd.h>
#include <utils/time/ticker.h>
#include "BaseStation.h"

// TODO: With multiagent use, a configuration or global positioning will need instead of this #def - possibly
//  over messaging. (local is quicker and easier to work with initially)
#define AGENT_X_OFFSET 0 // forward
#define AGENT_Y_OFFSET 2 // meters (right)
#define AGENT_Z_OFFSET 2 // meters (down)

namespace System
{
    BaseStation::BaseStation(): Runnable(1.0) // 1 hz
    {
        _communicator = new Messaging::BaseStationCommunicator(BASE_ID);
    }

    BaseStation::~BaseStation()
    {
        delete _communicator;
    }

    void BaseStation::doSetup()
    {
        // Subscribe to any specific messages
        _communicator->registerCallback(
                Messaging::Messages::Common::MessageID::MESSAGE_HELLO,
                [&](char * msg) { handleNewNode(msg); }
                );
        _communicator->registerCallback(
                Messaging::Messages::Common::MessageID::MESSAGE_ACK,
                [&](char * msg) { handleAck(msg); }
        );

        _communicator->registerCallback(
                Messaging::Messages::Common::MessageID::MESSAGE_AGENT_LOCATION,
                [&](char * msg) { handleAgentLocation(msg); }
        );

        _communicator->registerCallback(
                Messaging::Messages::Common::MessageID::MESSAGE_SUBJECT_LOCATION,
                [&](char * msg) { handleSubjectLocation(msg); }
        );
        _communicator->registerCallback(
                Messaging::Messages::Common::MessageID::MESSAGE_AGENT_MOVE_COMMAND,
                [&](char * msg) { handleAgentMoveCommand(msg); }
        );

        // Run the comms
        _communicator->setup();
        _communicator->run();
    }

    void BaseStation::doRun()
    {
        static auto msgTicker1 = new Utils::Time::Ticker(1, "Run");
        msgTicker1->tick();

        // Called specified rate in constructor

        // TODO: make this a better matching, for now we will use most recently communicated from and assume ther is only one controller at time active.
        // Match active agents with active controllers
        unsigned char bestController = 0;
        unsigned long bestControllerTime = 0;
        {
            std::unique_lock<std::mutex> lock1(_activeControllersMutex);
            // Find the most recent controller
            for (auto &controller: _activeControllers)
            {
                if (controller.second.lastMsgTime >= bestControllerTime)
                {
                    bestController = controller.first;
                    bestControllerTime = controller.second.lastMsgTime;
                }
            }
        }

        // Now we have the "best" controller...give it agent ownership
        if(_activeControllers.size() >= 1)
        {
            for (auto &agent: _activeAgents)
            {
                std::unique_lock<std::mutex> lock2(_agentOwnerMutex);
                auto ownerMapIter = _agentOwnerMap.find(agent.first);
                if (ownerMapIter == _agentOwnerMap.end())
                {
                    // No mapping for this agent, lets make one
                    std::cout << "Creating assignment for agent " << (int) agent.first << " to controller "
                              << (int) bestController << std::endl;
                    _agentOwnerMap.insert(std::make_pair(agent.first, bestController));
                } else
                {
                    if (ownerMapIter->second != bestController)
                    {
                        // We already have a mapping for this agent, lets update its controller
                        std::cout << "Re-assigning agent " << (int) agent.first << " to controller "
                                  << (int) bestController << std::endl;
                        ownerMapIter->second = bestController;
                    }
                }
            }
        }

        // Print out warnings for sizes
        {
            std::unique_lock<std::mutex> lock3(_activeAgentsMutex);
            if (_activeAgents.size() > 10)
            {
                std::cout << "WARN: Too many _activeAgents " << _activeAgents.size() << std::endl;
            }

            std::unique_lock<std::mutex> lock4(_agentOwnerMutex);
            if (_agentOwnerMap.size() > 10)
            {
                std::cout << "WARN: Too many _agentOwnerMap " << _agentOwnerMap.size() << std::endl;
            }

            std::unique_lock<std::mutex> lock5(_activeControllersMutex);
            if (_activeControllers.size() > 10)
            {
                std::cout << "WARN: Too many _activeControllers " << _activeControllers.size() << std::endl;
            }
        }

    }

    void BaseStation::doStop()
    {
        _communicator->stop();
        _running = false;
    }

    // -------------------------- //
    // ---- Message Callback ---- //
    // -------------------------- //
    void System::BaseStation::handleNewNode(char *helloMessage)
    {
        static auto msgTicker1 = new Utils::Time::Ticker(1, "");
        msgTicker1->tick("Start handle new node");

        // Cast the message
        auto hello = (Messaging::Messages::Common::Hello*) helloMessage;

        // Increment the node count for each new node
        char nodeId = NODE_ID_START + _nodeCount++;
        std::string ipAddr = std::to_string(hello->address.ip1) + "." + std::to_string(hello->address.ip2) + "." + std::to_string(hello->address.ip3) + "." + std::to_string(hello->address.ip4);

        // Mark this node down as an agent or a controller
        if(hello->node_type == NodeType::NODE_TYPE_AGENT)
        {
            std::cout << "Adding agent " << (int)nodeId << " to Active agents map." << std::endl;

            // Set the agent params - the origin will be set below on first location message after HELLO message
            AgentParams agentParams = {};
            agentParams.lastLocationUpdateTime = 0;
            agentParams.lastMessageTime = Utils::Time::microsNow();
            _activeAgents.emplace(std::make_pair(nodeId, agentParams));
        }
        else if(hello->node_type == NodeType::NODE_TYPE_CONTROLLER)
        {
            std::unique_lock<std::mutex> lock(_activeControllersMutex);

            std::cout << "Adding controller " << (int)nodeId << " to Active controller map." << std::endl;

            ControllerParams controllerParams = {};
            // TODO: These eventually will come from the hello message (likely) for now we assume same position as base.
            controllerParams.controllerToBaseOffset.x = 0;
            controllerParams.controllerToBaseOffset.y = 0;
            controllerParams.controllerToBaseOffset.z = 0;
            controllerParams.lastMsgTime = Utils::Time::microsNow();
            _activeControllers.emplace(std::make_pair(nodeId, controllerParams));
        }
        else
        {
            std::cout << "NACKing a welcome, invalid NODE_TYPE" << std::endl;

            // INVALID NODE TYPE, RESPOND WITH NACK
            _communicator->sendAck(nodeId, hello->header.message_id, hello->header.timestamp, false, ipAddr, hello->listeningPort);
            return;
        }

        // OTHERWISE, if we do not return due to an erroneous type above, lets send a WELCOME message

        // Send the welcome, join to network
        std::cout << "Joining " << ipAddr << ":" << hello->listeningPort << " to the network as a " <<
        (hello->node_type == NodeType::NODE_TYPE_AGENT ? "NODE_TYPE_AGENT": hello->node_type == NodeType::NODE_TYPE_CONTROLLER ? "NODE_TYPE_CONTROLLER": hello->node_type == NodeType::NODE_TYPE_BASE ? "NODE_TYPE_BASE" : "UNKNOWN TYPE")
        << " with ID " << (int) nodeId << std::endl;
        _communicator->sendWelcome(ipAddr, hello->listeningPort, nodeId);

        static auto msgTicker2 = new Utils::Time::Ticker(1, "", msgTicker1);
        msgTicker2->tick("Start handle new node");
    }

    void BaseStation::handleAck(char *ackMessage)
    {
        auto ack = (Ack*)ackMessage;
        std::cout << "Ack -- " << ack->message_id << " was " << (ack->ack_bool ? "ACKed": "NACKed") << " by " << ack->header.source_id << " at " << ack->acked_message_timestamp << std::endl;
    }

    void BaseStation::handleAgentLocation(char *agentLocationMessage)
    {
        static auto msgTicker1 = new Utils::Time::Ticker(1, "");
        msgTicker1->tick("Start agent location");

        // When we get a location, lets verify its from an agent....then lets forward it out to all the controllers

        auto agentLocation = (AgentLocation*) agentLocationMessage;

        // Check this is an agent
        if(!isAgent(agentLocation->header.source_id))
        {
            std::cout << "Error, agent location from non agent node" << std::endl;
            return;
        }

        // Inject ownership
        std::unique_lock<std::mutex> lock(_agentOwnerMutex);
        auto owneriter = _agentOwnerMap.find(agentLocation->agent_id);
        if(owneriter == _agentOwnerMap.end())
        {
            // std::cout << " ERROR: Cannot find the owner of the agent " << (int)agentLocation->agent_id << std::endl;
            return;
        }

        // Set the owner
        agentLocation->owner_id = owneriter->second;

        // Broad cast to all controllers
        std::unique_lock<std::mutex> lock2(_activeControllersMutex);
        for(auto controllerId: _activeControllers)
        {
            // Send the agent location out to each of the controllers registered
            _communicator->forwardAgentLocation(controllerId.first, agentLocation);
        }

        static auto msgTicker2 = new Utils::Time::Ticker(1, "", msgTicker1);
        msgTicker2->tick("End agent location");
    }

    void BaseStation::handleSubjectLocation(char *subjectLocationMessage)
    {

        // When we get a location, lets verify its from an agent....then lets forward it out to all the controllers

        auto subjectLocation = (SubjectLocation*) subjectLocationMessage;

        // Check this is an controller
        if(!isController(subjectLocation->header.source_id))
        {
            std::cout << "Error, subject location from non controller node" << std::endl;
            return;
        }

        // Broad cast to all controllers
        for(auto controllerId: _activeControllers)
        {
            // Send the subject location out to each of the controllers registered
            _communicator->forwardSubjectLocation(controllerId.first, subjectLocation);
        }
    }

    void BaseStation::handleAgentMoveCommand(char *agentMoveCommandMessage)
    {
        static auto msgTicker1 = new Utils::Time::Ticker(1, "");
        msgTicker1->tick("Start agent move command");

        // Lets make sure this move command 1) came from a controller, 2) came from a controller who owns the agent

        auto agentMoveCommand = (AgentMoveCommand*) agentMoveCommandMessage;

        // Check this is an controller
        if(!isController(agentMoveCommand->header.source_id))
        {
            std::cout << "Error, agent move command from non controller node" << std::endl;
            return;
        }

        // Mark this as an active
        {
            std::unique_lock<std::mutex> lock(_activeControllersMutex);
            auto activeControllerIter = _activeControllers.find(agentMoveCommand->header.source_id);
            if (activeControllerIter == _activeControllers.end())
            {
                std::cout << "Unknown active controller." << std::endl;
                return;
            }
            activeControllerIter->second.lastMsgTime = Utils::Time::microsNow();
        }

        // Make sure this controller owns control of this agent
        {
            std::unique_lock<std::mutex> lock(_agentOwnerMutex);
            auto agentOwnerItem = _agentOwnerMap.find(agentMoveCommand->agent_id);
            if (agentOwnerItem == _agentOwnerMap.end() || agentOwnerItem->second != agentMoveCommand->header.source_id)
            {
                std::cout << "Controller, " << (int) agentMoveCommand->header.source_id << " does not own "
                          << (int) agentMoveCommand->agent_id << std::endl;
                return;
            }
        }

        // Grab this controllers details
        std::unique_lock<std::mutex> lock(_activeControllersMutex);
        auto controller = _activeControllers.find(agentMoveCommand->header.source_id);
        if (controller == _activeControllers.end())
        {
            std::cout << "Controller not found, " << (int) agentMoveCommand->header.source_id << std::endl;
            return;
        }

        // Transform move to drone coordinates
        Location transformedLocation = {};
        transformedLocation.x = agentMoveCommand->target_location.x + controller->second.controllerToBaseOffset.x - AGENT_X_OFFSET;
        transformedLocation.y = agentMoveCommand->target_location.y + controller->second.controllerToBaseOffset.y - AGENT_Y_OFFSET;
        transformedLocation.z = agentMoveCommand->target_location.z + controller->second.controllerToBaseOffset.z - AGENT_Z_OFFSET;

        // Send to agent
        _communicator->sendAgentMove(agentMoveCommand->agent_id, transformedLocation);
        static auto msgTicker2 = new Utils::Time::Ticker(1, "", msgTicker1);
        msgTicker2->tick("End agent move command");
    }


    // TYPE checker helpers
    bool BaseStation::isAgent(char nodeId)
    {
        std::unique_lock<std::mutex> lock(_activeAgentsMutex);

        bool found = false;
        for(auto id: _activeAgents)
        {
            if(id.first == nodeId)
            {
                found = true;
                break;
            }
        }
        return found;
    }

    bool BaseStation::isController(char nodeId)
    {
        std::unique_lock<std::mutex> lock(_activeControllersMutex);

        bool found = false;
        for(auto id: _activeControllers)
        {
            if(id.first == nodeId)
            {
                found = true;
                break;
            }
        }
        return found;
    }

}
