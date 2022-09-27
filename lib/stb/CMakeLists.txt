cmake_minimum_required(VERSION 3.20.0)

add_library(stb_image src/stb_image.cpp include/stb/stb_image.h)

target_include_directories(stb_image INTERFACE include PRIVATE include/stb)

set_target_properties(stb_image PROPERTIES
    FOLDER "lib"
)
