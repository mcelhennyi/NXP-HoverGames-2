//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_DRONEMOVECOMMAND_H
#define HOVERGAMES2_DRONEMOVECOMMAND_H
#include "../common/header.h"
#include "../common/location.h"

namespace Messaging
{
    struct AgentMoveCommand
    {
        Header  header;
        Location target_location;      // The target location for the agent
    };
}

#endif //HOVERGAMES2_DRONEMOVECOMMAND_H
