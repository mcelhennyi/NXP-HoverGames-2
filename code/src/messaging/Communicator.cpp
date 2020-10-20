//
// Created by user on 10/17/20.
//

#include "Communicator.h"
#include "../utils/time.h"

namespace Messaging
{
    void Communicator::fillHeader(char targetId, MessageID messageIdEnum, Header *header)
    {
        header->message_id = messageIdEnum;
        header->target_id = targetId;
        header->source_id = _myId;
        header->timestamp = Utils::Time::microsNow();
    }

    // Private
    bool Communicator::sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, CommDetails *commDetails)
    {
        // TODO: Send the message
        return false;
    }

    // public
    bool Communicator::sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked)
    {
        auto commStruct = _nodes.find(targetId);
        if(commStruct != _nodes.end())
        {
            return false;
        }
        return sendAck(targetId, messageIdAcked, messageTimestamp, acked, &(commStruct->second));
    }

    // public
    bool Communicator::sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, std::string ipAddr, int port)
    {
        // Retain the network details for this node ID
        auto commStruct = _nodes.find(targetId);
        if(commStruct != _nodes.end())
        {
            _nodes.emplace(std::make_pair(targetId, CommDetails(ipAddr, port)));
        }
        return sendAck(targetId, messageIdAcked, messageTimestamp, acked, &(commStruct->second));
    }

    void Communicator::setupListeners()
    {

    }

    void Communicator::startListening()
    {

    }

    void Communicator::registerCallback(Messaging::Messages::Common::MessageID messageId,
                                        std::function<void(char *)> callback)
    {

    }

}

