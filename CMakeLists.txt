cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "comproenv")
project(PROJECT_NAME)

set(CMAKE_CXX_STANDARD 17)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
endif(MSVC)

include_directories("include")
include_directories("3rdparty")

add_subdirectory("3rdparty")
add_subdirectory("src")
add_subdirectory("application")
