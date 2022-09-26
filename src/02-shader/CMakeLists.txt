set(_src 
    main02.cpp Renderer.cpp Renderer.h Shader.h Shader.cpp
)
set(_shaders 
    shaders/v.shader shaders/f.shader
)
set(_all ${_src} ${_shaders})

add_executable(02 ${_all})

source_group(src FILES ${_src})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_shaders})

target_link_libraries(02
    glfw glad ${GLFW_LIBRARIES}
)

target_compile_definitions(02 PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
