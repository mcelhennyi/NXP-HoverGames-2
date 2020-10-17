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
                char ip1;
                char ip2;
                char ip3;
                char ip4;
            };

            struct Hello
            {
                Header header;
                char node_type;      // NodeType Enum above
                short listeningPort;  // The IP port this device listens on
                char _padding[5];
                IPAddress address;        // The IP address of this device
            };
        }
    }
}


#endif //HOVERGAMES2_HELLO_H
