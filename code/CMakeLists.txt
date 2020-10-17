cmake_minimum_required(VERSION 3.17)
project(HoverGames2)

set(CMAKE_CXX_STANDARD 14)

add_executable(HoverGames2 main.cpp src/messaging/messages/common/header.h src/messaging/messages/common/hello.h src/messaging/messages/common/welcome.h src/messaging/messages/common/ack.h src/messaging/messages/agent/agentLocation.h src/messaging/messages/controller/subjectLocation.h src/messaging/messages/common/location.h src/messaging/messages/controller/agentMoveCommand.h src/messaging/Communicator.cpp src/messaging/Communicator.h src/messaging/BaseStationCommunicator.cpp src/messaging/BaseStationCommunicator.h src/messaging/DroneCommunicator.cpp src/messaging/DroneCommunicator.h src/system/base_station/BaseStation.cpp src/system/base_station/BaseStation.h src/system/agent/Agent.cpp src/system/agent/Agent.h src/system/Runnable.cpp src/system/Runnable.h src/utils/time.h)