cmake_minimum_required(VERSION 3.20.0)



add_executable(01 main01.c)

target_link_libraries(01 
    glfw glad ${GLFW_LIBRARIES}
)
