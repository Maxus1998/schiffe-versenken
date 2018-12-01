cmake_minimum_required(VERSION 3.12)
project(schiffe_versenken)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.68.0)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(C:/boost/stage/lib)


add_executable(schiffe_versenken main.cpp)
add_executable(chat_client chat_client.cpp)
add_executable(chat_server chat_server.cpp)

target_link_libraries(schiffe_versenken ${Boost_LIBRARIES})
target_link_libraries(schiffe_versenken C:/boost/stage/lib/*)
target_link_libraries(chat_client C:/boost/stage/lib/*)
target_link_libraries(chat_server C:/boost/stage/lib/*)