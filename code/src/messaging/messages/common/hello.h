//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_HELLO_H
#define HOVERGAMES2_HELLO_H

#include "header.h"

namespace Messaging
{
    namespace Messages
    {
        namespace Common
        {
            enum NodeType
            {
                NODE_TYPE_BASE = 0,
                NODE_TYPE_AGENT,
                NODE_TYPE_CONTROLLER
            };

            struct IPAddress
            {
                unsigned char ip1;
                unsigned char ip2;
                unsigned char ip3;
                unsigned char ip4;
            };

            struct Hello
            {
                Header header;
                unsigned char node_type;        // NodeType Enum above
                unsigned short listeningPort;   // The IP port this device listens on
                unsigned char _padding[5];
                IPAddress address;              // The IP address of this device
            };
        }
    }
}


#endif //HOVERGAMES2_HELLO_H
