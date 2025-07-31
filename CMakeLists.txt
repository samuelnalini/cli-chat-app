cmake_minimum_required(VERSION 3.22)

# vcpkg

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")


project(ChatApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source Directories

set(SERVER_SOURCES server/src)
set(CLIENT_SOURCES client/src)
set(COMMON_SOURCES common/src)

# Dependencies

find_package(Curses REQUIRED)

# Server

add_executable(server
    ${SERVER_SOURCES}/main.cpp
    ${SERVER_SOURCES}/server.cpp
    ${COMMON_SOURCES}/common.cpp
    ${COMMON_SOURCES}/network_session.cpp
)

target_include_directories(server
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common/headers
)

target_link_libraries(server PRIVATE sodium)

# Client

add_executable(client
    ${CLIENT_SOURCES}/main.cpp
    ${CLIENT_SOURCES}/client.cpp
    ${CLIENT_SOURCES}/ncursesUI.cpp
    ${COMMON_SOURCES}/common.cpp
    ${COMMON_SOURCES}/network_session.cpp
)


target_link_libraries(client PRIVATE ${CURSES_LIBRARIES} sodium)
target_include_directories(client
    PRIVATE
        ${CURSES_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/common/headers
)
