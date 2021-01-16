cmake_minimum_required(VERSION 3.14)
project(HoverGames2)

set(CMAKE_CXX_STANDARD 14)

# Enable strict handling of warnings
add_definitions("-Wall -Wextra -Werror -Wno-reorder -Wno-unused-parameter")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/utils/thread/thread_pool)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/utils/mavsdk)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/utils/mavsdk)

add_executable(
        HoverGames2
        main.cpp
        src/messaging/messages/common/header.h
        src/messaging/messages/common/hello.h
        src/messaging/messages/common/welcome.h
        src/messaging/messages/common/ack.h
        src/messaging/messages/agent/agentLocation.h
        src/messaging/messages/controller/subjectLocation.h
        src/messaging/messages/common/location.h
        src/messaging/messages/controller/agentMoveCommand.h
        src/messaging/Communicator.cpp
        src/messaging/Communicator.h
        src/messaging/BaseStationCommunicator.cpp
        src/messaging/BaseStationCommunicator.h
        src/messaging/DroneCommunicator.cpp
        src/messaging/DroneCommunicator.h
        src/system/base_station/BaseStation.cpp
        src/system/base_station/BaseStation.h
        src/system/agent/Agent.cpp
        src/system/agent/Agent.h
        src/utils/thread/runnable/Runnable.cpp
        src/utils/thread/runnable/Runnable.h
        src/utils/util_time.h
        src/utils/thread/runnable/ThreadLoop.h)

TARGET_LINK_LIBRARIES(
    HoverGames2
    pthread
    mavsdk                  # All apps link against mavsdk library
    mavsdk_action           # If action plugin used by app ...
    mavsdk_telemetry        # If telemetry plugin used by app ...
    mavsdk_offboard         # If offboard plugin used by app ...
)