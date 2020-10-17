//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_ACK_H
#define HOVERGAMES2_ACK_H

#include "header.h"

namespace Messaging
{
    struct Ack
    {
        Header          header;
        unsigned long   acked_message_timestamp;    // The timestamp of the acked message
        char            message_id;                 // The acked message
        char            ack_bool;                   // 0 for false, anything else for true
    };
}

#endif //HOVERGAMES2_ACK_H
