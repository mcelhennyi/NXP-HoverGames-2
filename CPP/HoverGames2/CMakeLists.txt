cmake_minimum_required(VERSION 3.17)
project(HoverGames2)

set(CMAKE_CXX_STANDARD 14)

add_executable(HoverGames2 main.cpp src/messaging/common/header.h src/messaging/common/hello.h src/messaging/common/welcome.h src/messaging/common/ack.h src/messaging/controller/agentLocation.h src/messaging/controller/subjectLocation.h src/messaging/common/location.h src/messaging/controller/agentMoveCommand.h src/messaging/agent/droneMoveCommand.h)