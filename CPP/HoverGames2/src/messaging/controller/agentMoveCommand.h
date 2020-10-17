//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_AGENTMOVECOMMAND_H
#define HOVERGAMES2_AGENTMOVECOMMAND_H

#include "../common/header.h"
#include "../common/location.h"

namespace Messaging
{
    struct AgentMoveCommand
    {
        Header  header;
        char    agent_id;               // The ID of the agent
        char    _padding[7];            // Padding for alignment

        Location target_location;      // The target location for the agent
    };
}

#endif //HOVERGAMES2_AGENTMOVECOMMAND_H
