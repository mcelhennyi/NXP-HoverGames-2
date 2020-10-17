//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_AGENTLOCATION_H
#define HOVERGAMES2_AGENTLOCATION_H

#include "../common/header.h"
#include "../common/location.h"

namespace Messaging
{
    struct AgentLocation
    {
        Header  header;
        char    agent_id;               // The ID of the agent
        char    owner_id;               // The owner of the agent
        char    _padding[6];            // Padding for alignment

        Location current_location;      // The current location of the agent
        Location target_location;       // The target location of the agent
    };
}


#endif //HOVERGAMES2_AGENTLOCATION_H
