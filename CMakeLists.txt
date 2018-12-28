cmake_minimum_required(VERSION 3.12)
project(schiffe-versenken)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.68.0)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS}/stage/lib)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()


add_executable(schiffe-versenken main.cpp)
add_executable(server chat_server.cpp)
add_executable(client chat_client.cpp)
add_executable(test_client test_client.cpp)

target_link_libraries(schiffe-versenken ${Boost_LIBRARIES})
target_link_libraries(schiffe-versenken ${Boost_INCLUDE_DIRS}/stage/lib/*)
target_link_libraries(server ${Boost_INCLUDE_DIRS}/stage/lib/*)
target_link_libraries(client ${Boost_INCLUDE_DIRS}/stage/lib/*)
target_link_libraries(test_client ${Boost_INCLUDE_DIRS}/stage/lib/*)