cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

file(GLOB ASSIGNMENT_CPP_PATH ./*.cpp)
file(GLOB ASSIGNMENT_H_PATH ./*.hpp)

add_executable(RayTracing ${ASSIGNMENT_CPP_PATH} ${ASSIGNMENT_H_PATH})

