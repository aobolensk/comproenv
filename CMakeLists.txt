cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "comproenv")
project(PROJECT_NAME)

include_directories("include")
include_directories("3rdparty")

add_subdirectory("3rdparty/libyaml")
add_subdirectory("src")
add_subdirectory("application")
