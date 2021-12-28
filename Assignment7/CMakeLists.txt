cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file(GLOB ASSIGNMENT_CPP_PATH ./*.cpp)
file(GLOB ASSIGNMENT_H_PATH ./*.hpp)

add_executable(RayTracing ${ASSIGNMENT_CPP_PATH} ${ASSIGNMENT_H_PATH})

target_compile_options(RayTracing PUBLIC -pthread)
