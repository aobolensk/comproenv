file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "../include/*.h")

include("${PROJECT_SOURCE_DIR}/cmake/config.cmake")
add_library(comproenv-lib ${headers} ${sources})
target_link_libraries(comproenv-lib yaml)