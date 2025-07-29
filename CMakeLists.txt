cmake_minimum_required(VERSION 3.22)

project(ChatApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SERVER_SOURCES server/src)
set(CLIENT_SOURCES client/src)
set(COMMON_SOURCES common)

# Server

add_executable(server
    ${SERVER_SOURCES}/main.cpp
    ${SERVER_SOURCES}/server.cpp
    ${COMMON_SOURCES}/common.cpp
)

# Client

add_executable(client
    ${CLIENT_SOURCES}/main.cpp
    ${CLIENT_SOURCES}/client.cpp
    ${CLIENT_SOURCES}/ncursesUI.cpp
    ${COMMON_SOURCES}/common.cpp
)

find_package(Curses REQUIRED)
target_link_libraries(client PRIVATE ${CURSES_LIBRARIES})
target_include_directories(client PRIVATE ${CURSES_INCLUDE_DIR})
