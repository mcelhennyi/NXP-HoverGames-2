//
// Created by user on 10/17/20.
//

#include "BaseStationCommunicator.h"

namespace Messaging
{

    void BaseStationCommunicator::sendWelcome(std::string ipAddr, int port, char nodeId)
    {
        // Retain the network details for this node ID
        CommDetails details;
        auto commStruct = _nodes.find(nodeId);
        if(commStruct == _nodes.end())
        {
            details.ipAddr = ipAddr;
            details.port = port;
            _nodes.emplace(std::make_pair(nodeId, details));
        }
        else
        {
            details = commStruct->second;
        }

        Messaging::Messages::Common::Welcome welcome;
        fillHeader(nodeId, Messaging::Messages::Common::MessageID::MESSAGE_WELCOME, &welcome.header);
        welcome.node_id = nodeId;

        // Send out the welcome init message
        sendMessage(details, (char*)&welcome, sizeof(Welcome));
    }

    void BaseStationCommunicator::forwardAgentLocation(unsigned char targetId, AgentLocation *agentLocationMessage)
    {

    }

    void BaseStationCommunicator::forwardSubjectLocation(unsigned char targetId, SubjectLocation *subjectLocationMessage)
    {

    }

    void BaseStationCommunicator::sendAgentMove(unsigned char targetId, Location targetLoaction)
    {
        CommDetails details;
        auto commStruct = _nodes.find(targetId);
        if(commStruct != _nodes.end())
        {
            details = commStruct->second;
        }
        else
        {
            std::cout << "Unable to locate comm details for agent " << (int)targetId << std::endl;
            return;
        }

        AgentMoveCommand agentMoveCommand;
        fillHeader(targetId, MessageID::MESSAGE_AGENT_MOVE_COMMAND, &agentMoveCommand.header);
        agentMoveCommand.agent_id = targetId;
        agentMoveCommand.target_location = targetLoaction;
        sendMessage(details, (char*)&agentMoveCommand, sizeof(AgentMoveCommand));
    }
}

