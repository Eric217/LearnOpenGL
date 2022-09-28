set(_src 
    main03.cpp Renderer.cpp Renderer.h Shader.h Shader.cpp
    Texture.h Texture.cpp 
)
set(_shaders 
    shaders/v.shader shaders/f.shader
)
set(_texs 
    textures/container.jpg textures/awesomeface.png
)

set(_all ${_src} ${_shaders} ${_texs})

add_executable(03 ${_all})

source_group(src FILES ${_src})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_shaders} ${_texs})

target_link_libraries(03
    glfw glad stb_image glm ${GLFW_LIBRARIES}
)

target_compile_definitions(03 PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    TEXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/textures"
)
