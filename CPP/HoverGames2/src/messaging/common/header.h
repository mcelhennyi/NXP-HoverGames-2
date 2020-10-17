//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_HEADER_H
#define HOVERGAMES2_HEADER_H

namespace Messaging
{
    enum MessageID
    {
        // Common
        MESSAGE_HELLO=1,
        MESSAGE_WELCOME,
        MESSAGE_ACK,

        // Controller Messages
        MESSAGE_AGENT_LOCATION,
        MESSAGE_AGENT_MOVE_COMMAND,
        MESSAGE_SUBJECT_LOCATION,

        // Agent Messages
        MESSAGE_DRONE_MOVE_COMMAND,
        MESSAGE_CURRENT_LOCATION

    };

    struct Header
    {
        char            message_id;     // The message's id
        char            source_id;      // The Source of the message
        char            target_id=0;    // The target of the message - 255 for broadcast
        char            _padding[5];    // Unused - for alignment
        unsigned long   timestamp;      // This message creation time
    };
}

#endif //HOVERGAMES2_HEADER_H
