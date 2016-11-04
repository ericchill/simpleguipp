cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_BUILD_TYPE Debug)

add_definitions(-std=c++11)

add_executable(window window.cpp)

target_link_libraries(window glutpp GL X11 pthread)
