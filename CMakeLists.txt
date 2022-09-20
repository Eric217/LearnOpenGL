project(LearnOpenGL)

cmake_minimum_required(VERSION 3.20.0)

include_directories(
    ${GLFW_INCLUDE_DIRS}
)

link_directories(
    ${GLFW_LIBRARY_DIRS}
)

add_subdirectory(src)