project(LearnOpenGL)

cmake_minimum_required(VERSION 3.20.0)

find_package(OpenGL REQUIRED)
# find_package(Freetype REQUIRED)
find_package(PkgConfig QUIET)

add_subdirectory(glfw)
set_property(TARGET glfw APPEND_STRING PROPERTY COMPILE_FLAGS -w)

add_subdirectory(glad)
set_property(TARGET glad APPEND_STRING PROPERTY COMPILE_FLAGS -w)

add_subdirectory(src)
