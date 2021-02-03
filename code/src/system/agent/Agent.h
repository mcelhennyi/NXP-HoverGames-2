//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_AGENT_H
#define HOVERGAMES2_AGENT_H

#include <chrono>
#include <cstdint>
#include <mavsdk.h>

#include <utils/thread/runnable/Runnable.h>
#include <utils/thread/runnable/ThreadLoop.h>
#include <messaging/DroneCommunicator.h>
#include <messaging/messages/controller/agentMoveCommand.h>

#include <plugins/action/action.h>
#include <plugins/telemetry/telemetry.h>
#include <plugins/offboard/offboard.h>
#include <iostream>
#include <thread>

using namespace mavsdk;
using namespace std::this_thread;
using namespace std::chrono;


namespace System
{
    class Agent: public Runnable
    {
    public:
        enum AgentStateEnum{
            WAIT=0,
            TAKEOFF,
            TARGETING_MODE,
            RETURN_TO_LAUNCH,
            LAND
        };

        Agent();
        ~Agent();

        void doSetup() override;
        void doRun() override;
        void doStop() override;

    protected:
        // Position monitoring thread
        bool atPosition(Location target);

        // MAVSDK callbacks

        /// @brief Called when new components are discovered by the autopilot
        void componentDiscovered(ComponentType component_type);

        /// @brief Called when the position of the drone is sent
        void onNewPosition(Telemetry::PositionVelocityNed posvel);


        // Messaging callbacks
        void onAgentMoveCommand(char* agentMoveCommandMessage);

    private:
        // Communicator to talk out of this process
        Messaging::DroneCommunicator        *_communicator;

        // Mavsdk vars
        Mavsdk                              _mavsdk;
        std::shared_ptr<mavsdk::System>     _system;
        std::shared_ptr<mavsdk::Telemetry>  _telemetry;
        std::shared_ptr<mavsdk::Action>     _action;
        std::shared_ptr<mavsdk::Offboard>   _offboard;

        // Drone state
        std::mutex                          _currentPositionMutex;
        Location                            _currentPosition;
        std::atomic_bool                    _armed;
        std::mutex                          _droneStateMutex;
        Telemetry::LandedState              _droneState;
        std::atomic_bool                    _takeoffCommanded;

        // States
        AgentStateEnum                      _agentState;
        AgentStateEnum                      _lastAgentState;
        std::mutex                          _agentStateMutex;
        std::condition_variable             _agentStateCV;

        // Target following
        std::atomic_bool                    _staleTarget;
        bool                                _lastTargetTime;
        std::mutex                          _targetCommandMutex;
        uint64_t                            _newTargetReceivedTimeUs;
        AgentMoveCommand                    _newMoveCommand;
        AgentMoveCommand                    _lastMoveCommand;
        int                                 _atTargetCounter;

        // Home
        std::mutex                          _homeLocationsMutex;
        Location                            _homeLocationGround;
        Location                            _homeLocationAir;

    };
}

#endif //HOVERGAMES2_AGENT_H
