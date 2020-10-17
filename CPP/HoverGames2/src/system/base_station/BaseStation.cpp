//
// Created by user on 10/17/20.
//

#include <iostream>
#include <unistd.h>
#include "BaseStation.h"

namespace System
{
    void BaseStation::setup()
    {
        // Start anything that needs to be kicked off
        _communicator->setupListeners();

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
    }

    void BaseStation::run()
    {
        _running = true;

        while(_running)
        {
            // Do coordinated logic here
            usleep(1000000 * 100);
        }
    }

    // -------------------------- //
    // ---- Message Callback ---- //
    // -------------------------- //
    void System::BaseStation::handleNewNode(char *helloMessage)
    {
        // Cast the message
        auto hello = (Messaging::Messages::Common::Hello*) helloMessage;

        // Increment the node count for each new node
        char nodeId = NODE_ID_START + _nodeCount++;
        std::string ipAddr = std::to_string(hello->address.ip1) + std::to_string(hello->address.ip2) + std::to_string(hello->address.ip3) + std::to_string(hello->address.ip4);

        // Mark this node down as an agent or a controller
        if(hello->node_type == NodeType::NODE_TYPE_AGENT)
        {
            _activeAgents.emplace_back(nodeId);
        }
        else if(hello->node_type == NodeType::NODE_TYPE_CONTROLLER)
        {
            _activeControllers.emplace_back(nodeId);
        }
        else
        {
            // INVALID NODE TYPE, RESPOND WITH NACK
            _communicator->sendAck(nodeId, hello->header.message_id, hello->header.timestamp, false, ipAddr, hello->listeningPort);
            return;
        }

        // OTHERWISE, if we do not return due to an erroneous type above, lets send a WELCOME message

        // Send the welcome, join to network
        _communicator->sendWelcome(ipAddr, hello->listeningPort, nodeId);
    }

    void BaseStation::handleAck(char *ackMessage)
    {
        auto ack = (Ack*)ackMessage;
        std::cout << "Ack -- " << ack->message_id << " was " << (ack->ack_bool ? "ACKed": "NACKed") << " by " << ack->header.source_id << " at " << ack->acked_message_timestamp << std::endl;
    }

    void BaseStation::handleAgentLocation(char *agentLocationMessage)
    {
        // When we get a location, lets verify its from an agent....then lets forward it out to all the controllers

        auto agentLocation = (AgentLocation*) agentLocationMessage;

        // Check this is an agent
        if(!isAgent(agentLocation->header.source_id))
        {
            std::cout << "Error, agent location from non agent node" << std::endl;
        }

        // Broad cast to all controllers
        for(auto controllerId: _activeControllers)
        {
            // Send the agent location out to each of the controllers registered
            _communicator->forwardAgentLocation(controllerId, agentLocation);
        }
    }

    void BaseStation::handleSubjectLocation(char *subjectLocationMessage)
    {
        // When we get a location, lets verify its from an agent....then lets forward it out to all the controllers

        auto subjectLocation = (SubjectLocation*) subjectLocationMessage;

        // Check this is an controller
        if(!isController(subjectLocation->header.source_id))
        {
            std::cout << "Error, subject location from non controller node" << std::endl;
        }

        // Broad cast to all controllers
        for(auto controllerId: _activeControllers)
        {
            // Send the subject location out to each of the controllers registered
            _communicator->forwardSubjectLocation(controllerId, subjectLocation);
        }
    }

    void BaseStation::handleAgentMoveCommand(char *agentMoveCommandMessage)
    {
        // Lets make sure this move command 1) came from a controller, 2) came from a controller who owns the agent

        auto agentMoveCommand = (AgentMoveCommand*) agentMoveCommandMessage;

        // Check this is an controller
        if(!isController(agentMoveCommand->header.source_id))
        {
            std::cout << "Error, agent move command from non controller node" << std::endl;
        }

        // Broad cast to all controllers
        for(auto controllerId: _activeControllers)
        {
            // Send the subject location out to each of the controllers registered
            _communicator->forwardSubjectLocation(controllerId, subjectLocation);
        }
    }

    bool BaseStation::isAgent(char nodeId)
    {
        bool found = false;
        for(auto id: _activeAgents)
        {
            if(id == nodeId)
            {
                found = true;
                break;
            }
        }
        return found;
    }

    bool BaseStation::isController(char nodeId)
    {
        bool found = false;
        for(auto id: _activeControllers)
        {
            if(id == nodeId)
            {
                found = true;
                break;
            }
        }
        return found;
    }

}
