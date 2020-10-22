//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_HEADER_H
#define HOVERGAMES2_HEADER_H

namespace Messaging
{
    namespace Messages
    {
        namespace Common
        {
            enum MessageID
            {
                // Common
                MESSAGE_ACK=1,

                // Node ORIGINATING message (Agent or Controller)
                MESSAGE_HELLO,

                // Base Station ORIGINATING messages
                MESSAGE_WELCOME,

                // Controller ORIGINATING Messages
                MESSAGE_AGENT_MOVE_COMMAND,
                MESSAGE_SUBJECT_LOCATION,

                // Agent ORIGINATING Messages
                MESSAGE_AGENT_LOCATION

            };

            struct Header
            {
                unsigned char   message_id;     // The message's id
                unsigned char   source_id;      // The Source of the message
                unsigned char   target_id=0;    // The target of the message - 255 for broadcast
                unsigned char   _padding[5];    // Unused - for alignment
                unsigned long   timestamp;      // This message creation time
            };
        }
    }
}

#endif //HOVERGAMES2_HEADER_H
