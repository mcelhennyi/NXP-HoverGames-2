//
// Created by user on 10/17/20.
//

#include "Agent.h"

#include <messaging/messages/agent/agentLocation.h>

#define CONNECTION_URL "127.0.0.1:5760"

#define SET_POINT_TIMEOUT (uint64_t)((1.0 / 5.0) * 1000000)  // 5Hz worth of timeout (we expect 10 HZ) - converted to micros

namespace System
{
    Agent::Agent(): Runnable(10), _targetThread(std::bind(&Agent::monitorPosition, this), 10), _flyToTarget(false), _lastTargetTime(false), _currentState(AgentStateEnum::WAIT)
    {

    }

    Agent::~Agent()
    {

    }

    void Agent::doSetup()
    {
        // First setup our internal comms
        _communicator->registerCallback(
                Messaging::Messages::Common::MessageID::MESSAGE_AGENT_MOVE_COMMAND,
                [&](char * msg) { onAgentMoveCommand(msg); }
        );

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
        _telemetry->subscribe_position(
            std::bind(&Agent::onNewPosition, this, std::placeholders::_1)
        );


    }

    void Agent::doRun()
    {
        switch (_currentState)
        {
            case WAIT:

                break;
            case TAKEOFF:

                break;
            case TARGETING_MODE:

                break;
            case RETURN_TO_LAUNCH:

                break;
            case LAND:

                break;
        }
    }

    void Agent::doStop()
    {

    }

    void Agent::monitorPosition()
    {
        // This function loops at a rate of 10 hz

        // Check for an expiration
        auto timeNow = Utils::Time::microsNow();
        if(timeNow - _newTargetReceivedTimeUs > SET_POINT_TIMEOUT)
        {
            // We have timed out (the time between now and the last message is too great
            
        }

    }

    // MavSDK callbacks
    void Agent::componentDiscovered(ComponentType component_type)
    {
        std::cout << "Discovered a component with type " << unsigned(component_type) << std::endl;
    }

    void Agent::onNewPosition(Telemetry::Position position)
    {
        std::cout << "Altitude: " << position.relative_altitude_m << " m" << std::endl;
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

    }
    // End Messaging Callbacks
}