//
// Created by user on 10/17/20.
//

#include "DroneCommunicator.h"

namespace Messaging
{
    DroneCommunicator::DroneCommunicator(): Communicator(/* Unknown id, will be assigned */), _baseStationListen("0.0.0.0", 12345)
    {

    }

    void DroneCommunicator::sendHello()
    {
        auto helloMessage = Hello{};
        fillHeader(0, MessageID::MESSAGE_HELLO, &helloMessage.header);
        helloMessage.address.ip1 = 0; // TODO: Link to _listenDetails
        helloMessage.address.ip2 = 0; // TODO: Link to _listenDetails
        helloMessage.address.ip3 = 0; // TODO: Link to _listenDetails
        helloMessage.address.ip4 = 0; // TODO: Link to _listenDetails
        helloMessage.listeningPort = _listenDetails.port;
        helloMessage.node_type = NodeType::NODE_TYPE_AGENT;
        sendMessage(_baseStationListen, (char*)&helloMessage, sizeof(Hello));
    }
}

