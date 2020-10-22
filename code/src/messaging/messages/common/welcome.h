//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_WELCOME_H
#define HOVERGAMES2_WELCOME_H

#include "header.h"

namespace Messaging
{
    namespace Messages
    {
        namespace Common
        {
            struct Welcome
            {
                Header header;
                unsigned char node_id;    // The assigned ID
            };
        }
    }
}


#endif //HOVERGAMES2_WELCOME_H
