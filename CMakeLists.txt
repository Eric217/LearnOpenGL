project(LearnOpenGL)

cmake_minimum_required(VERSION 3.20.0)

find_package(OpenGL REQUIRED)
# find_package(Freetype REQUIRED)
find_package(PkgConfig QUIET)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS ON)

add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(lib)
add_subdirectory(submodules)

set_property(TARGET glfw glad stb_image assimp APPEND_STRING PROPERTY 
    COMPILE_FLAGS " -w"
)

set_property(TARGET assimp glm_shared unit PROPERTY
    FOLDER "lib"
)

add_subdirectory(src)
