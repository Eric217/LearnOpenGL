cmake_minimum_required(VERSION 3.20.0)

set(_src main01.cpp Renderer.cpp Renderer.h)
add_executable(01 ${_src})

source_group(src FILES ${_src})

target_link_libraries(01 
    glfw glad ${GLFW_LIBRARIES}
)
