//
// Created by user on 10/17/20.
//

#include "BaseStationCommunicator.h"

namespace Messaging
{

    void BaseStationCommunicator::sendWelcome(std::string ipAddr, int port, char nodeId)
    {
        // Retain the network details for this node ID
        auto commStruct = _nodes.find(nodeId);
        if(commStruct != _nodes.end())
        {
            _nodes.emplace(std::make_pair(nodeId, CommDetails(ipAddr, port)));
        }

        Messaging::Messages::Common::Welcome welcome;
        fillHeader(nodeId, Messaging::Messages::Common::MessageID::MESSAGE_WELCOME, &welcome.header);
        welcome.node_id = nodeId;

        // Send out the welcome init message
        // TODO TX message

    }

    void BaseStationCommunicator::forwardAgentLocation(char targetId, AgentLocation *agentLocationMessage)
    {

    }

    void BaseStationCommunicator::forwardSubjectLocation(char targetId, SubjectLocation *subjectLocationMessage)
    {

    }

    void BaseStationCommunicator::sendAgentMove(char targetId, Location targetLoaction)
    {

    }
}

