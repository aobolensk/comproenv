file(GLOB_RECURSE sources "*.cpp")

include("${PROJECT_SOURCE_DIR}/cmake/config.cmake")
add_executable(comproenv main.cpp)
if (MSVC)
    target_link_libraries(comproenv comproenv-lib)
elseif(MSVC)
    target_link_libraries(comproenv comproenv-lib stdc++ stdc++fs)
endif(MSVC)