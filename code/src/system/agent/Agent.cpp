//
// Created by user on 10/17/20.
//

#include "Agent.h"
#include <utils/thread/runnable/ThreadLoop.h>

#include <messaging/messages/agent/agentLocation.h>
#include <utils/time/ticker.h>

#define CONNECTION_URL "tcp://127.0.0.1:5760"
#define SAFE_ALTITUDE -2 // (NED) The altitude to fly at - this overrides all Zs sent.
#define SET_POINT_TIMEOUT 1000000 * 2 // 2 seconds // TODO PUT BACK(uint64_t)((1.0 / 5.0) * 1000000)  // 5Hz worth of timeout (we expect 10 HZ) - converted to micros
#define DISTANCE_THRESHOLD 0.2 // meters
#define SETTLE_TIMES 10

namespace System
{
    Agent::Agent(): Runnable(10), _lastTargetTime(false), _agentState(AgentStateEnum::WAIT)
    {
        _communicator = new Messaging::DroneCommunicator();
        _positionSendingThread = new Utils::Thread::ThreadLoop(std::bind(&Agent::sendPositionToGround, this), 5);
    }

    Agent::~Agent()
    {
        delete _communicator;
    }

    void Agent::doSetup()
    {
        _positionSendingThread->setup();

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
        for(auto &system: _mavsdk.systems())
        {
            if(system->get_system_id() == 1) // grab the ap
            {
                _system = system;
                break;
            }
        }

        // Register a callback so we get told when components (camera, gimbal) etc
        // are found.
        _system->register_component_discovered_callback(
            std::bind(&Agent::componentDiscovered, this, std::placeholders::_1)
        );

        // Grab the telem and action classes
        _telemetry = std::make_shared<Telemetry>(_system);
        _action = std::make_shared<Action>(_system);

        // Subscribe to telemetry mesages
        _telemetry->set_rate_position_velocity_ned(5);
        _telemetry->subscribe_position_velocity_ned(
            std::bind(&Agent::onNewPosition, this, std::placeholders::_1)
        );

        _telemetry->set_rate_landed_state(5);
        _telemetry->subscribe_landed_state([&](Telemetry::LandedState landedState){
            std::unique_lock<std::mutex> lock(_droneStateMutex);

            if(_droneState!= landedState)
                std::cout << "Drone is in LandedState: " <<
                          (landedState == Telemetry::LandedState::TakingOff ? "TakingOff" :
                           landedState == Telemetry::LandedState::InAir ? "InAir" :
                           landedState == Telemetry::LandedState::Landing ? "Landing" :
                           landedState == Telemetry::LandedState::OnGround ? "OnGround" :
                           landedState == Telemetry::LandedState::Unknown ? "Unknown" : "") << std::endl;

            _droneState = landedState;
        });

        _telemetry->subscribe_armed([&](bool armed){
            if(_armed != armed)
                std::cout << "Drone is " << (armed ? "Armed": "Disarmed") << std::endl;

            _armed = armed;
        });

        _telemetry->subscribe_flight_mode([&](Telemetry::FlightMode flightMode){
            std::unique_lock<std::mutex> lock(_flightModeMutex);
            _flightMode = flightMode;
        });

        // Grab the offboard controller
        _offboard = std::make_shared<Offboard>(_system);

    }

    void Agent::doRun()
    {
        // static auto msgTicker1 = new Utils::Time::Ticker(1, "Start Run");
        // msgTicker1->tick();

        _positionSendingThread->run();

        // This function loops at a rate of 10 hz

        // Aquire locks
        std::unique_lock<std::mutex> agentStateLock(_agentStateMutex);
        std::unique_lock<std::mutex> droneStateLock(_droneStateMutex);

        AgentStateEnum nextState = _agentState; // default to current state

        // Check for an expiration
        auto timeNow = Utils::Time::microsNow();
        if(timeNow - _newTargetReceivedTimeUs > SET_POINT_TIMEOUT)
        {
            // We have timed out (the time between now and the last message is too great)
            if(!_staleTarget)
                std::cout << "AgentMoveCommand expired, too much time since last message." << std::endl;
            _staleTarget = true;

            // Make sure we do a shutdown sequence when we stop getting messages
            if(_agentState == AgentStateEnum::TAKEOFF || _agentState == AgentStateEnum::TARGETING_MODE)
            {
                static auto rtlTicker = new Utils::Time::Ticker(1, "");
                rtlTicker->tick("Commanding RTL due to message timeout.");
                _agentState = AgentStateEnum::RETURN_TO_LAUNCH;  // Here we set directly
                nextState = _agentState;  // Here we set directly
            }
        }
        else
        {
            _staleTarget = false;
        }

        if(_agentState != _lastAgentState)
            std::cout << "AGENT STATE: " <<
                                         (_agentState == AgentStateEnum::WAIT ? "WAIT":
                                         _agentState == AgentStateEnum::LAND ? "LAND":
                                         _agentState == AgentStateEnum::RETURN_TO_LAUNCH ? "RETURN_TO_LAUNCH":
                                         _agentState == AgentStateEnum::TAKEOFF ? "TAKEOFF":
                                         _agentState == AgentStateEnum::TARGETING_MODE ? "TARGETING_MODE": "UNKNOWN")
                    << std::endl;

        switch (_agentState)
        {
            case WAIT:
            {
                // Lock and wait for a non stale target
                droneStateLock.unlock();
                _agentStateCV.wait(agentStateLock, [&](){ return !(_staleTarget.load()); });
                droneStateLock.lock();

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
                agentStateLock.unlock();
                if ((_droneState == Telemetry::LandedState::OnGround && !_takeoffCommanded) || !_armed)
                {
                    droneStateLock.unlock();

                    // First ARM
                    if(!_armed)
                    {
                        std::cout << "Sending arm command..." << std::endl;
                        _takeoffCommanded = false;
                        auto result = _action->arm();
                        if (result != Action::Result::Success)
                        {
                            std::cout << "Error: Failed to arm: " << result << std::endl;
                            break;
                        }
                    }

                    // Send takeoff command
                    // std::cout << "Setting takeoff altitude..." << std::endl;
                    auto result = _action->set_takeoff_altitude(-SAFE_ALTITUDE);
                    if (result == Action::Result::Success)
                    {
                        std::cout << "Sending takeoff command..." << std::endl;
                        result = _action->takeoff();
                        if (result != Action::Result::Success)
                        {
                            _takeoffCommanded = false;
                            std::cout << "Error: Failed to takeoff: " << result << std::endl;
                        }
                        _takeoffCommanded = true;
                    }
                    else
                    {
                        _takeoffCommanded = false;
                        std::cout << "Error: Failed to set takeoff altitude: " << result << std::endl;
                    }

                }
                // else if (_droneState == Telemetry::LandedState::TakingOff)  // TODO: This state is NEVER reported
                // {
                //     // DO nothing, we are waiting
                // }
                // else if (_droneState == Telemetry::LandedState::InAir)
                // {
                //     nextState = AgentStateEnum::TARGETING_MODE;
                //     _takeoffCommanded = false;
                // }
                // else
                // {
                //     std::cout << "Waiting on takeoff sequence to complete" << std::endl;
                // }

                static auto takeoffTicker = new Utils::Time::Ticker(1, "Waiting on takeoff to complete...");
                takeoffTicker->tick();

                // Check the position, we want to be at our takeoff location (some height above the ground point)
                // TODO: Monitor for position - this may not be needed if all we need is the INAIR state.
                // std::cout << "Waiting for takeoff to complete..." << std::endl;
                if(atPosition(_homeLocationAir, true))
                {
                    std::cout << "Drone at position, moving on." << std::endl;
                    _takeoffCommanded = false;
                    nextState = AgentStateEnum::TARGETING_MODE;
                }
                break;
            }
            case TARGETING_MODE:
            {
                agentStateLock.unlock();
                droneStateLock.unlock();

                std::unique_lock<std::mutex> targetLocationLock(_targetCommandMutex);

                // Send the command for the target location to the autopilot
                Offboard::PositionNedYaw positionNedYaw;
                positionNedYaw.north_m = _newMoveCommand.target_location.x;
                positionNedYaw.east_m = _newMoveCommand.target_location.y;
                positionNedYaw.down_m = SAFE_ALTITUDE;
                positionNedYaw.yaw_deg = 0;
                targetLocationLock.unlock();

                // std::cout << "Setting set position for offboard control: (NED) " << positionNedYaw.north_m << ", " << positionNedYaw.east_m << ", " << positionNedYaw.down_m  << std::endl;
                auto result = _offboard->set_position_ned(positionNedYaw);
                if(result != Offboard::Result::Success)
                {
                    std::cout << "Error: Failed to change set point in RETURN_TO_LAUNCH mode, result: " << result << std::endl;
                }
                else
                {
                    std::unique_lock<std::mutex> lock(_flightModeMutex);

                    // std::cout << "Offboard mode is " << _offboard->is_active() << std::endl;
                    // std::cout << "Flight mode is offboard: " << (_flightMode == Telemetry::FlightMode::Offboard) << std::endl;
                    if(_flightMode != Telemetry::FlightMode::Offboard)
                    {
                        lock.unlock();

                        std::cout << "Starting offboard control..." << std::endl;
                        result = _offboard->start();
                        if (result != Offboard::Result::Success)
                        {
                            std::cout << "Error: Failed to start offboard mode: " << result << std::endl;
                        }
                    }
                }
                break;
            }
            case RETURN_TO_LAUNCH:
            {
                agentStateLock.unlock();
                droneStateLock.unlock();

                std::unique_lock<std::mutex> homeLocationLock(_homeLocationsMutex);

                // Command a return to home
                Offboard::PositionNedYaw positionNedYaw;
                positionNedYaw.north_m = _homeLocationAir.x;
                positionNedYaw.east_m = _homeLocationAir.y;
                positionNedYaw.down_m = _homeLocationAir.z;
                positionNedYaw.yaw_deg = 0;
                // std::cout << "Setting position for return to launch." << std::endl;
                auto result = _offboard->set_position_ned(positionNedYaw);
                if(result != Offboard::Result::Success)
                {
                    std::cout << "Error: Failed to change set point in RETURN_TO_LAUNCH mode, result: " << result << std::endl;
                }

                static auto rtlTicker = new Utils::Time::Ticker(1, "Returning home...");
                rtlTicker->tick();

                // Monitor position till we are near launch, then land.
                if(atPosition(_homeLocationAir)|| _droneState == Telemetry::LandedState::OnGround)
                {
                    // Turn off offboard mode
                    // std::cout << "Disabling offboard mode..."<<std::endl;
                    // result = _offboard->stop();
                    // TODO: COmmand denied when sending this stop (change to HOLD) command...why???
                    // if(result == Offboard::Result::Success)
                    // {
                    //     std::cout << "Offboard mode disabled, LANDing"<<std::endl;
                        nextState = AgentStateEnum::LAND;
                    // }
                    // else
                    // {
                    //     std::cout << "Error: Failed to turn off offboard mode, result: " << result << std::endl;
                    // }
                }
                break;
            }
            case LAND:
            {
                agentStateLock.unlock();

                if(_droneState != Telemetry::LandedState::Landing && _droneState != Telemetry::LandedState::OnGround)
                {
                    droneStateLock.unlock();

                    // Send land command
                    std::cout << "Sending land command to AP..." << std::endl;
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
                    // std::cout << "Disarming AP..." << std::endl;
                    static auto disarmingTicker = new Utils::Time::Ticker(1, "Disarming...");
                    disarmingTicker->tick();
                    // auto result = _action->disarm();  // TODO: This blocks for ever for some reason...
                    // if(result != Action::Result::Success)
                    if(_armed)
                    {
                        // std::cout << "Error: Failed to command a disarm, result: " << result << std::endl;
                    }
                    else
                    {
                        // Success, lets WAIT
                        std::cout << "Disarmed." << std::endl;
                        nextState = AgentStateEnum::WAIT;
                    }
                }

                break;
            }
        }

        if(!agentStateLock.owns_lock())
        {
            agentStateLock.lock();
        }
        _lastAgentState = _agentState;
        _agentState = nextState;

        // static auto msgTicker2 = new Utils::Time::Ticker(1, "End Run", msgTicker1);
        // msgTicker2->tick();

    }

    void Agent::doStop()
    {
        _positionSendingThread->stop();

        std::unique_lock<std::mutex> droneStateLock(_droneStateMutex);
        while(_droneState != Telemetry::LandedState::OnGround)
        {
            if(_droneState == Telemetry::LandedState::InAir)
            {
                droneStateLock.unlock();

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


    bool Agent::atPosition(Location target, bool zOnly)
    {
        std::unique_lock<std::mutex> lock(_currentPositionMutex);

        // First calculate the distance to the target
        double d = 0;

        if(zOnly)
        {
            d = std::abs(target.z - _currentPosition.z);
        }
        else
        {
            d = sqrt(pow(target.x - _currentPosition.x, 2) +
                     pow(target.y - _currentPosition.y, 2) +
                     pow(target.z - _currentPosition.z, 2) * 1.0);
        }

        // Now compare to distance threshold
        if(d <= DISTANCE_THRESHOLD)
        {
            // Increment the success counter
            ++_atTargetCounter;
            return _atTargetCounter >= SETTLE_TIMES;
        }

        // Reset the success counter
        _atTargetCounter = 0;

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
        // static auto msgTicker1 = new Utils::Time::Ticker(1, "New position");
        // msgTicker1->tick();

        std::unique_lock<std::mutex> lock(_currentPositionMutex);
        _currentPosition.x = posvel.position.north_m;
        _currentPosition.y = posvel.position.east_m;
        _currentPosition.z = posvel.position.down_m;
    }
    // End MavSDK callbacks

    // Messaging Callbacks
    void Agent::onAgentMoveCommand(char* agentMoveCommandMessage)
    {
        // static auto msgTicker1 = new Utils::Time::Ticker(1, "Start agent move");
        // msgTicker1->tick();

        std::unique_lock<std::mutex> actionStateLock(_agentStateMutex);
        _staleTarget = false;
        _agentStateCV.notify_all();
        actionStateLock.unlock();

        std::unique_lock<std::mutex> lock(_targetCommandMutex);
        _newTargetReceivedTimeUs = Utils::Time::microsNow();

        auto casted = (AgentMoveCommand*) agentMoveCommandMessage;
        if(casted->target_location.x == _newMoveCommand.target_location.x && casted->target_location.y == _newMoveCommand.target_location.y && casted->target_location.z == _newMoveCommand.target_location.z)
        {
            // static auto msgTicker2 = new Utils::Time::Ticker(1, "End agent move", msgTicker1);
            // msgTicker2->tick();
            return;
        }
        else
        {
            std::cout << "New set point" << std::endl;
        }

        // Save off the old command
        _lastMoveCommand = _newMoveCommand;

        // copy to our class var (this gets deleted once this callback is over)
        _newMoveCommand = *((AgentMoveCommand*) agentMoveCommandMessage);
        // static auto msgTicker2 = new Utils::Time::Ticker(1, "End agent move", msgTicker1);
        // msgTicker2->tick();
    }

    void Agent::sendPositionToGround()
    {
        // static auto msgTicker1 = new Utils::Time::Ticker(1, "Send position to ground");
        // msgTicker1->tick();

        std::unique_lock<std::mutex> lock1(_currentPositionMutex);
        std::unique_lock<std::mutex> lock2(_targetCommandMutex);

        Location currentLocation = _currentPosition;
        Location targetLocation = _newMoveCommand.target_location;

        lock1.unlock();
        lock2.unlock();

        _communicator->sendAgentLocation(currentLocation, targetLocation);
    }
    // End Messaging Callbacks
}