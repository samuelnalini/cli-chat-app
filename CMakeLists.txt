cmake_minimum_required(VERSION 3.22)

# vcpkg

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")


project(ChatApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source Directories

set(SERVER_SOURCES server/src)
set(CLIENT_SOURCES client/src)
set(COMMON_SOURCES common/src)

# Dependencies

set(CURSES_NEED_WIDE TRUE)
find_library(NCURSESW_LIBRARY ncursesw REQUIRED)
find_path(NCURSESW_INCLUDE_DIR ncurses.h REQUIRED)

# Server

add_executable(server
    ${SERVER_SOURCES}/main.cpp
    ${SERVER_SOURCES}/server.cpp
    ${COMMON_SOURCES}/network_session.cpp
    ${COMMON_SOURCES}/debug.cpp
    ${COMMON_SOURCES}/style.cpp
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
    ${COMMON_SOURCES}/network_session.cpp
    ${COMMON_SOURCES}/debug.cpp
    ${COMMON_SOURCES}/style.cpp
)


target_include_directories(client
    PRIVATE
        ${NCURSESW_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/external/utf8cpp/source
        ${CMAKE_SOURCE_DIR}/common/headers
)
target_link_libraries(client PRIVATE ${NCURSESW_LIBRARY} sodium)
