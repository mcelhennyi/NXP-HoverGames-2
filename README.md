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
2) The Base station
3) The Controller(s) - a node.

There can be many agents, and many controllers, however there is only one centralized base station. A controller runs the RL algorthm and classifier, and is in ownership and control of the agents in the sceme. An agent is owned by one controller. The base station acts as a router/conduit and rule enforcer between the controllers and the agents. The base station is responsible for setting up a network ontop of UDP/TCP depending on the configuration. This network has a defined set of messages that can pass through it. When a new node is created, it asks the base station for an ID and lets the base station know what type of node it is (Agent or controller).

Our drone software is a connector from the autopilot to our base station that is running our reinforcement learning algorithm and our classifier. As the classifier neural network detects animals (using humans as a stand in for testing) it fills a scene with `subjects`. Meanwhile the drone is constantly broadcasting its location to the various controllers by way of the base station. Then the RL algorithm runs over the scene, processing the locations of the `subjects` and the drones to optimize the commanded position of the drone. The ground station then generates a target location for the drone and the drone then takes off and flys to that location. As the drone travels to the location our RL algorithm constantly tracks the animals and the drone which enables the algorithm to change instructions on the fly. Then as the drone approaches the animals, they are scared away and out of the keep out zone.

## Related Repositories
[Here](https://github.com/ajberlier/SCAREcrow) you can find our RL + Neural Network related code for 1) detecting animals and 2) Directing agents (drones) to herd the detected animals.

## Notes
Note about my findings [here](https://github.com/mcelhennyi/NXP-HoverGames-2/blob/master/NOTES.md)
