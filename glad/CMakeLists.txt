cmake_minimum_required(VERSION 3.20.0)

add_library(glad glad.c glad/glad.h KHR/khrplatform.h)
target_include_directories(glad PUBLIC ./)
