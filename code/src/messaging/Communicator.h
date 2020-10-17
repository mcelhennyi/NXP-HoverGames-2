//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_COMMUNICATOR_H
#define HOVERGAMES2_COMMUNICATOR_H

#include <functional>
#include <map>

#include "../utils/thread/threadPool/includes/static_pool.hpp"

#include "messages/common/hello.h"
#include "messages/common/welcome.h"
#include "messages/common/ack.h"

#include "messages/agent/agentLocation.h"

#include "messages/controller/agentMoveCommand.h"
#include "messages/controller/subjectLocation.h"

using namespace Messaging::Messages::Common;
using namespace Messaging::Messages::Agent;
using namespace Messaging::Messages::Controller;

#define CALLBACK_HANDLER_THREAD_COUNT 10  // note work is done in these threads, so we may need more.

namespace Messaging
{
    class Communicator
    {
    public:
        Communicator(char myId): _myId(myId), _threadPool(CALLBACK_HANDLER_THREAD_COUNT) {};
        ~Communicator() {};

        void setId(char id) { _myId = id; };

        void setupListeners();
        void startListening();

        void registerCallback(Messaging::Messages::Common::MessageID messageId, std::function<void(char*)> callback);

        // SEND
        /// Used when the target has already been sent to, will return false if target details cannot be found
        bool sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked);

        /// Used when the target has not already been sent to
        bool sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, std::string ipAddr, int port);

    protected:
        struct CommDetails
        {
            CommDetails(std::string &ipAddr_, int port_): ipAddr(ipAddr_), port(port_) {};
            std::string ipAddr;
            int port;
        };

        // PREPARE
        void fillHeader(char targetId, MessageID messageIdEnum, Header *header);


    private:

        /// Called by protected sendAcks once connections details are created/found
        bool sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, CommDetails* commDetails);

    protected:
        // Track the node locations
        std::map<char, CommDetails> _nodes;

        // Track our ID for message header use
        char _myId;

        // Use a thread pool for call back handling
        thread_pool::static_pool    _threadPool;

    };
}



#endif //HOVERGAMES2_COMMUNICATOR_H
