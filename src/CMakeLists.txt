file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "../include/*.h")

include("${PROJECT_SOURCE_DIR}/cmake/config.cmake")

# Get comproenv commit hash
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" OUTPUT_VARIABLE comproenv_hash)
string(STRIP ${comproenv_hash} comproenv_hash)
add_definitions(-DCOMPROENV_HASH=${comproenv_hash})

# Get libyaml commit hash
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/3rdparty/libyaml" OUTPUT_VARIABLE libyaml_hash)
string(STRIP ${libyaml_hash} libyaml_hash)
add_definitions(-DCOMPROENV_LIBYAML_HASH=${libyaml_hash})

# Get build timestamp
string(TIMESTAMP build_time "%Y-%m-%d %H:%M:%S" UTC)
add_definitions(-DCOMPROENV_BUILDTIME=${build_time})

add_library(comproenv-lib ${headers} ${sources})
target_link_libraries(comproenv-lib yaml)
set_target_properties(comproenv-lib PROPERTIES OUTPUT_NAME comproenv)
if (MSVC)
    target_compile_options(comproenv-lib PRIVATE "/MP")
endif (MSVC)
