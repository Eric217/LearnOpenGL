set(_target_name 07-stencil)

set(_src 
    main07.cpp Renderer.cpp Renderer.h Shader.h Shader.cpp
    Texture.h Texture.cpp Camera.h Camera.cpp
    Light.h Light.cpp Model.hpp Model.cpp
)

file(GLOB_RECURSE _shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)

file(GLOB_RECURSE _models ${CMAKE_CURRENT_SOURCE_DIR}/models/*)

set(_all ${_src} ${_shaders} ${_models})

add_executable(${_target_name} ${_all})

source_group(src FILES ${_src})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_shaders} ${_models})

target_link_libraries(${_target_name}
    glfw glad stb_image glm assimp ${GLFW_LIBRARIES}
)

target_compile_definitions(${_target_name} PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    MODEL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/models"
    TARGET_NAME="${_target_name}"
)
