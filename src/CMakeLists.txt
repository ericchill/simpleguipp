cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES "*.cpp")

set(LIB_SOURCES ${SOURCES})
list(REMOVE_ITEM LIB_SOURCES tester.cpp)

add_definitions(-std=c++11)

add_library(glutpp SHARED ${LIB_SOURCES})
add_library(glutpptest STATIC ${LIB_SOURCES})

set_target_properties(glutpp PROPERTIES
  VERSION 0.0.1
  SOVERSION 0
  )

add_executable(tester tester.cpp ${LIB_SOURCES})
target_link_libraries(tester GL X11 pthread)

install(TARGETS glutpp DESTINATION lib)
