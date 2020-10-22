//
// Created by user on 10/17/20.
//

#include <iostream>
#include <unistd.h>
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

    }

    BaseStation::~BaseStation()
    {

    }

    void BaseStation::doSetup()
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

    void BaseStation::doRun()
    {
        // Called specified rate in constructor
        std::cout << "RUN!" << std::endl;
    }

    void BaseStation::doStop()
    {
        _running = false;
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
            // Set the agent params - the origin will be set below on first location message after HELLO message
            AgentParams agentParams = {};
            agentParams.lastLocationUpdateTime = 0;
            _activeAgents.emplace(std::make_pair(nodeId, agentParams));
        }
        else if(hello->node_type == NodeType::NODE_TYPE_CONTROLLER)
        {
            ControllerParams controllerParams = {};
            // TODO: These eventually will come from the hello message (likely) for now we assume same position as base.
            controllerParams.controllerToBaseOffset.x = 0;
            controllerParams.controllerToBaseOffset.y = 0;
            controllerParams.controllerToBaseOffset.z = 0;
            _activeControllers.emplace(std::make_pair(nodeId, controllerParams));
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
            return;
        }

        // Broad cast to all controllers
        for(auto controllerId: _activeControllers)
        {
            // Send the agent location out to each of the controllers registered
            _communicator->forwardAgentLocation(controllerId.first, agentLocation);
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
        // Lets make sure this move command 1) came from a controller, 2) came from a controller who owns the agent

        auto agentMoveCommand = (AgentMoveCommand*) agentMoveCommandMessage;

        // Check this is an controller
        if(!isController(agentMoveCommand->header.source_id))
        {
            std::cout << "Error, agent move command from non controller node" << std::endl;
            return;
        }

        // Make sure this controller owns control of this agent
        auto agentOwnerItem = _agentOwnerMap.find(agentMoveCommand->agent_id);
        if(agentOwnerItem == _agentOwnerMap.end() || agentOwnerItem->second != agentMoveCommand->header.source_id)
        {
            std::cout << "Controller, " << agentMoveCommand->header.source_id << " does not own " << agentMoveCommand->agent_id << std::endl;
            return;
        }

        // Grab this controllers details
        auto controller = _activeControllers.find(agentMoveCommand->header.source_id);
        if(controller != _activeControllers.end())
        {
            std::cout << "Controller not found, " <<  agentMoveCommand->header.source_id << std::endl;
        }

        // Transform move to drone coordinates
        Location transformedLocation = {};
        transformedLocation.x = agentMoveCommand->target_location.x + controller->second.controllerToBaseOffset.x - AGENT_X_OFFSET;
        transformedLocation.y = agentMoveCommand->target_location.y + controller->second.controllerToBaseOffset.y - AGENT_Y_OFFSET;
        transformedLocation.z = agentMoveCommand->target_location.z + controller->second.controllerToBaseOffset.z - AGENT_Z_OFFSET;

        // Send to agent
        _communicator->sendAgentMove(agentMoveCommand->agent_id, transformedLocation);

    }


    // TYPE checker helpers
    bool BaseStation::isAgent(char nodeId)
    {
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
