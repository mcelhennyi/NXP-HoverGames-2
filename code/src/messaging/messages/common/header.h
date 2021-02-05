//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_HEADER_H
#define HOVERGAMES2_HEADER_H

#include <iostream>

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

            inline std::string messageIdToString(MessageID messageId)
            {
                switch(messageId)
                {
                    case MessageID::MESSAGE_ACK:
                        return "MESSAGE_ACK";
                    case MessageID::MESSAGE_HELLO:
                        return "MESSAGE_HELLO";
                    case MessageID::MESSAGE_WELCOME:
                        return "MESSAGE_WELCOME";
                    case MessageID::MESSAGE_AGENT_MOVE_COMMAND:
                        return "MESSAGE_AGENT_MOVE_COMMAND";
                    case MessageID::MESSAGE_SUBJECT_LOCATION:
                        return "MESSAGE_SUBJECT_LOCATION";
                    case MessageID::MESSAGE_AGENT_LOCATION:
                        return "MESSAGE_AGENT_LOCATION";
                    default:
                        return "UNKNOWN MESSAGE";
                }

            }

            inline std::ostream& operator<<(std::ostream& stream, const MessageID& messageId)
            {
                return stream << messageIdToString(messageId);
            }

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
