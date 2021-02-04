//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_COMMUNICATOR_H
#define HOVERGAMES2_COMMUNICATOR_H

#include <utils/thread/thread_pool/includes/static_pool.hpp>
#include <utils/util_time.h>

#include <utils/thread/runnable/Runnable.h>

#include <messaging/messages/common/hello.h>
#include <messaging/messages//common/welcome.h>
#include <messaging/messages//common/ack.h>

#include <messaging/messages//agent/agentLocation.h>

#include <messaging/messages//controller/agentMoveCommand.h>
#include <messaging/messages//controller/subjectLocation.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <functional>
#include <map>

using namespace Messaging::Messages::Common;
using namespace Messaging::Messages::Agent;
using namespace Messaging::Messages::Controller;

#define CALLBACK_HANDLER_THREAD_COUNT 1  // note work is done in these threads, so we may need more.
#define MAXLINE     1024

namespace Messaging
{
    class Communicator: public Runnable
    {
    public:
        Communicator(std::string baseIp="0.0.0.0", int port=12345, unsigned char myId=0);
        ~Communicator();

        //void setId(char id) { _myId = id; };

        void registerCallback(Messaging::Messages::Common::MessageID messageId, std::function<void(char*)> callback);

        // SEND
        /// Used when the target has already been sent to, will return false if target details cannot be found
        bool sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked);

        /// Used when the target has not already been sent to
        bool sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, std::string ipAddr, int port);

    protected:
        struct CommDetails
        {
            CommDetails(){};
            CommDetails(std::string ipAddr_, int port_): ipAddr(ipAddr_), port(port_) {};
            std::string ipAddr;
            int port;
        };

        void doSetup() override;
        void doRun() override;
        void doStop() override;

        // PREPARE
        void fillHeader(unsigned char targetId, MessageID messageIdEnum, Header *header);
        void sendMessage(CommDetails &commDetails, char* message, int length);

        // Addr port conversions
        sockaddr_in convertToCStruct(CommDetails& commDetails);

    private:

        /// Called by protected sendAcks once connections details are created/found
        bool sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, CommDetails &commDetails);

        void callbackWrapper(std::function<void(char*)> func, char* buffer);

    protected:
        // Receiver stuff
        CommDetails                                         _listenDetails;
        int                                                 _sockfdReceive;
        int                                                 _sockfdSend;
        char                                                _buffer[MAXLINE];

        unsigned long                                       _lastTimeReceived;

        // Track the node locations
        std::map<unsigned char, CommDetails>                _nodes;

        // Track our ID for message header use
        unsigned char                                       _myId;
        unsigned char                                       _baseId;

        // Use a thread pool for call back handling
        std::map<unsigned char, thread_pool::static_pool*>  _threadPoolMap;

        // Registration of callbacks - msg_id:callback
        std::map<unsigned char, std::function<void(char*)>> _callbacks;

    };
}



#endif //HOVERGAMES2_COMMUNICATOR_H
