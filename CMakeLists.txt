cmake_minimum_required(VERSION 3.22)

project(ChatApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SERVER_SOURCES server/src)
set(CLIENT_SOURCES client/src)

add_executable(server
    ${SERVER_SOURCES}/main.cpp
    ${SERVER_SOURCES}/server.cpp
    common/common.cpp
)

add_executable(client
    ${CLIENT_SOURCES}/main.cpp
    ${CLIENT_SOURCES}/client.cpp
    common/common.cpp
)
