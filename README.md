# NXP HoverGames Challenge 2: Help Drones, Help Others During Pandemics with NXP
![](https://github.com/mcelhennyi/NXP-HoverGames-2/blob/master/SCAREcrow_logo.png)

## Our goal
Wild animals destroy property, kill/infect livestock, as well as harm agriculture. We will create a drone that can herd wild animals, preventing such harm.

## Our Submission
See it [here](https://www.hackster.io/contests/hovergames2/hardware_applications/12777)!

## Whats in this repo?
In this repo you will find our C++ code for our drone and our ground station.

## How does our software work?
Our system has three first class citizens:  
1) The Agent(s) (drone) - a node.
2) The Base station - the central, main, node.
3) The Controller(s) - a node.

The scene consists of two players:  
1) Agents - drones to direct to herd animals out of zone.
2) Subjects - animals to keep out of keep out zone.

There can be many agents, and many controllers, however there is only one centralized base station. A controller runs the RL algorthm and `subject` classifier, and is in ownership and control of the agents in the `scene`. An `agent` is owned by one `controller`. The `base station` acts as a router/conduit and rule enforcer between the `controllers` and the `agents`. The `base station` is responsible for setting up a network on top of UDP/TCP over WiFi depending on the configuration. This network has a defined set of messages that can pass through it. When a new node is created, it asks the `base station` for an ID and lets the base station know what type of node it is (Agent or controller). Once the agent has been assigned a `node ID` it is able to begin its life.

The RL algorithm and the animal classifier/tracker run on the `controller` node. As the classifier neural network detects `subjects` (using humans as a stand in for testing) it fills the RL scene with `subjects`. Additionally a `controller` can share its `subjects` with other `controllers` by sending a message to the `base station`. Meanwhile the drone is constantly broadcasting its location to the various controllers by way of the `base station`. The RL algorithm runs over the scene, processing the locations of the `subjects` and the `agents` to optimize the commanded position of the `agents` it owns. The `base station` then generates a target location for the `agent` and the `agent` then takes off and flys to that location. As the `agent` travels to the location our RL algorithm constantly tracks the `subjects` and the `agent` which enables the algorithm to change instructions on the fly. Then as the `agent` approaches the `subjects` they are scared away and out of the keep out zone. Once the `subject` of interest has left the keep out zone the `agent` lands back in its starting position and awaits new instructions.

## Related Repositories
[Here](https://github.com/ajberlier/SCAREcrow) you can find our RL + Neural Network related code for:  
1) detecting animals and  
2) Directing agents (drones) to herd the detected animals.

## Notes
Note about my findings [here](https://github.com/mcelhennyi/NXP-HoverGames-2/blob/master/NOTES.md)
