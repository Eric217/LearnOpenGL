set(_target_name 12-shadow)

set(_src 
    main12.cpp Renderer.cpp Renderer.h Shader.h Shader.cpp Texture.h Camera.h
    Camera.cpp Config.hpp Config.cpp Texture.cpp Light.h Light.cpp Model.hpp
    Model.cpp UBO.hpp UBO.cpp Framebuffer.hpp Framebuffer.cpp Scene.hpp
    Scene.cpp
)

file(GLOB_RECURSE _shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*)

file(GLOB_RECURSE _models ${CMAKE_CURRENT_SOURCE_DIR}/models/*)

list(FILTER _models EXCLUDE REGEX "\.DS_Store")
list(FILTER _shaders EXCLUDE REGEX "\.DS_Store")

set(_all ${_src} ${_shaders} ${_models})

set(CMAKE_CXX_STANDARD 20)

add_executable(${_target_name} ${_all})

source_group(src FILES ${_src})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_shaders} ${_models})

target_link_libraries(${_target_name}
    glfw glad stb_image glm assimp ${GLFW_LIBRARIES}
)

target_compile_definitions(${_target_name} PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    MODEL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/models"
    GLOBAL_MODEL_DIR="${GLOBAL_MODEL_DIR}"
    TARGET_NAME="${_target_name}"
)
