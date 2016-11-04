cmake_minimum_required(VERSION 3.5.1)

project(glutpp)

set(glutpp VERSION_MAJOR 0)
set(glutpp VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

include_directories(include)
link_directories(lib)

add_subdirectory(progs)
add_subdirectory(src)
add_subdirectory(freeglut)
