//
// Created by user on 10/17/20.
//

#include "Agent.h"

#include <messaging/messages/agent/agentLocation.h>

#define CONNECTION_URL "tcp://127.0.0.1:5760"
#define SAFE_ALTITUDE -2 // (NED) The altitude to fly at - this overrides all Zs sent.
#define SET_POINT_TIMEOUT (uint64_t)((1.0 / 5.0) * 1000000)  // 5Hz worth of timeout (we expect 10 HZ) - converted to micros
#define DISTANCE_THRESHOLD 0.2 // meters
#define SETTLE_TIME_SECONDS 2

namespace System
{
    Agent::Agent(): Runnable(10), _lastTargetTime(false), _agentState(AgentStateEnum::WAIT)
    {
        _communicator = new Messaging::DroneCommunicator();
    }

    Agent::~Agent()
    {
        delete _communicator;
    }

    void Agent::doSetup()
    {
        // Turn on Comms
        _communicator->setup();
        _communicator->run();

        std::mutex              connectedToBaseMutex;
        std::atomic_bool        connectedToBase(false);
        std::condition_variable connectedToBaseCv;

        // First setup our internal comms
        _communicator->registerCallback(Messaging::Messages::Common::MessageID::MESSAGE_AGENT_MOVE_COMMAND, [&](char * msg) {
            onAgentMoveCommand(msg);
        });
        _communicator->registerCallback(Messaging::Messages::Common::MessageID::MESSAGE_WELCOME, [&](char * msg) {
            // Trigger below to continue on
            std::lock_guard<std::mutex> baseConnectionLock(connectedToBaseMutex);
            connectedToBase = true;
            connectedToBaseCv.notify_all();
        });

        // Send a hello
        _communicator->sendHello();

        // Wait for connection
        // TODO: Add a timeout with a stop() call
        std::unique_lock<std::mutex> baseConnectionLock(connectedToBaseMutex);
        connectedToBaseCv.wait(baseConnectionLock, [&](){ return connectedToBase.load(); });

        // Now Setup the MAVSDK connections
        bool discoveredSystem = false;

        // Connect to the autopilot
        ConnectionResult connectionResult = _mavsdk.add_any_connection(CONNECTION_URL);
        if (connectionResult != ConnectionResult::Success)
        {
            std::cout << "Connection failed: " << connectionResult << std::endl;
            stop(); // Force a stop
        }

        // Discover the system
        std::cout << "Waiting to discover system..." << std::endl;
        _mavsdk.subscribe_on_new_system([&]() {
            const auto system = _mavsdk.systems().at(0);

            if (system->is_connected()) {
                std::cout << "Discovered system" << std::endl;
                discoveredSystem = true;
            }
        });

        // We usually receive heartbeats at 1Hz, therefore we should find a system after around 2
        // seconds.
        sleep_for(seconds(2));

        if (!discoveredSystem) {
            std::cout << "No system found, exiting." << std::endl;
            stop(); // Force a stop
        }

        // Capture the system
        _system = _mavsdk.systems().at(0);

        // Register a callback so we get told when components (camera, gimbal) etc
        // are found.
        _system->register_component_discovered_callback(
            std::bind(&Agent::componentDiscovered, this, std::placeholders::_1)
        );

        // Grab the telem and action classes
        _telemetry = std::make_shared<Telemetry>(_system);
        _action = std::make_shared<Action>(_system);

        // Subscribe to telemetry mesages
        _telemetry->set_rate_position(40);
        _telemetry->subscribe_position_velocity_ned(
            std::bind(&Agent::onNewPosition, this, std::placeholders::_1)
        );

        _telemetry->set_rate_landed_state(10);
        _telemetry->subscribe_landed_state([&](Telemetry::LandedState landedState){
            std::unique_lock<std::mutex> lock(_droneStateMutex);
            _droneState = landedState;

            std::cout << "Drone is in LandedState: " <<
                (_droneState == Telemetry::LandedState::TakingOff ? "TakingOff" :
                    _droneState == Telemetry::LandedState::InAir ? "InAir" :
                    _droneState == Telemetry::LandedState::Landing ? "Landing" :
                    _droneState == Telemetry::LandedState::OnGround ? "OnGround" :
                    _droneState == Telemetry::LandedState::Unknown ? "Unknown" : "") << std::endl;

        });

        _telemetry->subscribe_armed([&](bool armed){
            _armed = armed;
            std::cout << "Drone is " << (armed ? "Armed": "Disarmed") << std::endl;
        });

    }

    void Agent::doRun()
    {
        // This function loops at a rate of 10 hz

        // Aquire locks
        std::unique_lock<std::mutex> agentStateLock(_agentStateMutex);
        std::unique_lock<std::mutex> droneStateLock(_droneStateMutex);

        // Check for an expiration
        auto timeNow = Utils::Time::microsNow();
        if(timeNow - _newTargetReceivedTimeUs > SET_POINT_TIMEOUT)
        {
            // We have timed out (the time between now and the last message is too great)
            std::cout << "AgentMoveCommand expired, too much time since last message." << std::endl;
            _staleTarget = true;
        }
        else
        {
            _staleTarget = false;
        }

        AgentStateEnum nextState = AgentStateEnum::WAIT;
        switch (_agentState)
        {
            case WAIT:
            {
                // Lock and wait for a non stale target
                _agentStateCV.wait(agentStateLock, [&](){ return !(_staleTarget.load()); });

                // The target is not stale, lets go after it (takeoff first)
                if (!_staleTarget)
                {
                    if (_droneState == Telemetry::LandedState::OnGround)
                    {
                        nextState = AgentStateEnum::TAKEOFF;
                        _homeLocationGround = _currentPosition;
                        _homeLocationAir = _homeLocationGround;
                        _homeLocationAir.z = SAFE_ALTITUDE;
                    }
                    else
                    {
                        std::cout << "ERROR: Drone not on the ground, cannot move from WAIT to TAKEOFF" << std::endl;
                    }
                }
                break;
            }
            case TAKEOFF:
            {
                if (_droneState == Telemetry::LandedState::OnGround)
                {
                    // First ARM
                    auto result = _action->arm();
                    if (result == Action::Result::Success)
                    {
                        // Send takeoff command
                        result = _action->set_takeoff_altitude(SAFE_ALTITUDE);
                        if (result == Action::Result::Success)
                        {
                            result = _action->takeoff();
                            if (result != Action::Result::Success)
                            {
                                std::cout << "Error: Failed to takeoff: " << result << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << "Error: Failed to set takeoff altitude: " << result << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Error: Failed to arm: " << result << std::endl;
                    }
                }
                else if (_droneState == Telemetry::LandedState::TakingOff)
                {
                    // DO nothing, we are waiting
                }
                else if (_droneState == Telemetry::LandedState::InAir)
                {
                    nextState = AgentStateEnum::TARGETING_MODE;
                }

                // Check the position, we want to be at our takeoff location (some height above the ground point)
                // // TODO: Monitor for position - this may not be needed if all we need is the INAIR state.
                // if(atPosition(_homeLocationAir))
                // {
                //     nextState = AgentStateEnum::TARGETING_MODE;
                // }
                break;
            }
            case TARGETING_MODE:
            {
                std::unique_lock<std::mutex> targetLocationLock(_targetCommandMutex);

                // Send the command for the target location to the autopilot
                Offboard::PositionNedYaw positionNedYaw;
                positionNedYaw.north_m = _newMoveCommand.target_location.x;
                positionNedYaw.east_m = _newMoveCommand.target_location.y;
                positionNedYaw.down_m = SAFE_ALTITUDE;
                positionNedYaw.yaw_deg = 0;
                auto result = _offboard->set_position_ned(positionNedYaw);
                if(result != Offboard::Result::Success)
                {
                    std::cout << "Error: Failed to change set point in RETURN_TO_LAUNCH mode, result: " << result << std::endl;
                }
                else
                {
                    result = _offboard->start();
                    if(result != Offboard::Result::Success)
                    {
                        std::cout << "Error: Failed to start offboard mode: " << result << std::endl;
                    }
                }
                break;
            }
            case RETURN_TO_LAUNCH:
            {
                std::unique_lock<std::mutex> targetLocationLock(_targetCommandMutex);

                // Command a return to home
                Offboard::PositionNedYaw positionNedYaw;
                positionNedYaw.north_m = _homeLocationAir.x;
                positionNedYaw.east_m = _homeLocationAir.y;
                positionNedYaw.down_m = _homeLocationAir.z;
                positionNedYaw.yaw_deg = 0;
                auto result = _offboard->set_position_ned(positionNedYaw);
                if(result != Offboard::Result::Success)
                {
                    std::cout << "Error: Failed to change set point in RETURN_TO_LAUNCH mode, result: " << result << std::endl;
                }

                // Monitor position till we are near launch, then land.
                if(atPosition(_homeLocationAir))
                {
                    // Turn off offboard mode
                    result = _offboard->stop();
                    if(result == Offboard::Result::Success)
                    {
                        _agentState = AgentStateEnum::LAND;
                    }
                    else
                    {
                        std::cout << "Error: Failed to turn off offboard mode, result: " << result << std::endl;
                    }
                }
                break;
            }
            case LAND:
            {
                if(_droneState == Telemetry::LandedState::InAir)
                {
                    // Send land command
                    auto result = _action->land();
                    if(result != Action::Result::Success)
                    {
                        std::cout << "Error: Failed to command a land, result: " << result << std::endl;
                    }
                }
                else if(_droneState == Telemetry::LandedState::Landing)
                {
                    // Do nothing, just wait for landed
                }
                else if(_droneState == Telemetry::LandedState::OnGround)
                {
                    // Now disarm and wait
                    auto result = _action->disarm();
                    if(result != Action::Result::Success)
                    {
                        std::cout << "Error: Failed to command a disarm, result: " << result << std::endl;
                    }
                    else
                    {
                        // Success, lets WAIT
                        nextState = AgentStateEnum::WAIT;
                    }
                }

                break;
            }
        }

        _agentState = nextState;
    }

    void Agent::doStop()
    {
        std::unique_lock<std::mutex> droneStateLock(_droneStateMutex);
        while(_droneState != Telemetry::LandedState::OnGround)
        {
            if(_droneState == Telemetry::LandedState::InAir)
            {
                // Send land command
                auto result = _action->land();
                if(result != Action::Result::Success)
                {
                    std::cout << "Error: Failed to command a land, result: " << result << std::endl;
                }
            }
            else if(_droneState == Telemetry::LandedState::Landing)
            {
                // Wait for landed
            }
            else if(_droneState == Telemetry::LandedState::OnGround)
            {
                // Now disarm and wait
                auto result = _action->disarm();
                if(result != Action::Result::Success)
                {
                    std::cout << "Error: Failed to command a disarm, result: " << result << std::endl;
                }
                else
                {
                    // We are safe and can stop.
                }
                break;
            }
            usleep(1000000 / 10); // roughly 10hz
        }
    }


    bool Agent::atPosition(Location target)
    {
        std::unique_lock<std::mutex> lock(_currentPositionMutex);

        // First calculate the distance to the target
        double d = sqrt(pow(target.x - _currentPosition.x, 2) +
                       pow(target.y - _currentPosition.y, 2) +
                       pow(target.z - _currentPosition.z, 2) * 1.0);

        // Now compare to distance threshold
        if(d <= DISTANCE_THRESHOLD)
        {
            // Lets sleep some time for settling
            usleep(1000000 * SETTLE_TIME_SECONDS);
            return true;
        }

        // Delay for some time to settle, before allowing a state transition.
        return false;
    }

    // MavSDK callbacks
    void Agent::componentDiscovered(ComponentType component_type)
    {
        std::cout << "Discovered a component with type " << unsigned(component_type) << std::endl;
    }

    void Agent::onNewPosition(Telemetry::PositionVelocityNed posvel)
    {
        std::unique_lock<std::mutex> lock(_currentPositionMutex);
        _currentPosition.x = posvel.position.north_m;
        _currentPosition.y = posvel.position.east_m;
        _currentPosition.z = posvel.position.down_m;
    }
    // End MavSDK callbacks

    // Messaging Callbacks
    void Agent::onAgentMoveCommand(char* agentMoveCommandMessage)
    {
        std::unique_lock<std::mutex> lock(_targetCommandMutex);

        // Save off the old command
        _lastMoveCommand = _newMoveCommand;

        // copy to our class var (this gets deleted once this callback is over)
        _newTargetReceivedTimeUs = Utils::Time::microsNow();
        _newMoveCommand = *((AgentMoveCommand*) agentMoveCommandMessage);

        // std::cout << "Agent move command from " << (int)_newMoveCommand.header.source_id << std::endl;

        // Mark this as NOT stale
        std::lock_guard<std::mutex> actionStateLock(_agentStateMutex);
        _staleTarget = false;
        _agentStateCV.notify_all();

    }
    // End Messaging Callbacks
}