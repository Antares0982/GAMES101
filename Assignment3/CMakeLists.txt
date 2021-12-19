cmake_minimum_required(VERSION 3.10)

project(Rasterizer)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(/usr/local/include ./include)

file(GLOB ASSIGNMENT_CPP_PATH ./*.cpp)
file(GLOB ASSIGNMENT_H_PATH ./*.hpp)

add_executable(Rasterizer ${ASSIGNMENT_CPP_PATH} ${ASSIGNMENT_H_PATH})

target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})

target_compile_options(Rasterizer PUBLIC -Wall -Wextra -Wno-narrowing -pedantic)
