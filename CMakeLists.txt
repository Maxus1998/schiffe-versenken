cmake_minimum_required(VERSION 3.12)
project(schiffe-versenken)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.68.0)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS}/stage/lib)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()


add_executable(client client.cpp)
add_executable(server server.cpp Schiffe_versenken.cpp Schiffe_versenken.hpp)


target_link_libraries(client ${Boost_LIBRARIES})
target_link_libraries(client ${Boost_INCLUDE_DIRS}/stage/lib/*)
target_link_libraries(server ${Boost_INCLUDE_DIRS}/stage/lib/*)