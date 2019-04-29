file(GLOB_RECURSE sources "*.cpp")

include("${PROJECT_SOURCE_DIR}/cmake/config.cmake")
add_executable(comproenv main.cpp)
target_link_libraries(comproenv comproenv-lib stdc++fs)