project(LearnOpenGL)

cmake_minimum_required(VERSION 3.20.0)

find_package(OpenGL REQUIRED)
# find_package(Freetype REQUIRED)
find_package(PkgConfig QUIET)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(lib)
add_subdirectory(submodules)

set_property(TARGET glfw glad stb_image APPEND_STRING PROPERTY 
    COMPILE_FLAGS -w
)

add_subdirectory(src)
