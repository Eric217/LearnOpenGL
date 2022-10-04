set(_target_name 04-camera)

set(_src 
    main04.cpp Renderer.cpp Renderer.h Shader.h Shader.cpp
    Texture.h Texture.cpp Camera.h Camera.cpp
)
set(_shaders 
    shaders/v.shader shaders/f.shader
)
set(_texs 
    textures/container.jpg textures/awesomeface.png
)

set(_all ${_src} ${_shaders} ${_texs})

add_executable(${_target_name} ${_all})

source_group(src FILES ${_src})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_shaders} ${_texs})

target_link_libraries(${_target_name}
    glfw glad stb_image glm ${GLFW_LIBRARIES}
)

target_compile_definitions(${_target_name} PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    TEXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/textures"
)
