cmake_minimum_required(VERSION 3.22)

project(ChatApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

add_executable(server
    server/src/main.cpp
    server/src/server.cpp
    common/common.cpp
)

add_executable(client
    client/src/main.cpp
    client/src/client.cpp
    common/common.cpp
)
