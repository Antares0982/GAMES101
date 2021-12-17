cmake_minimum_required(VERSION 3.10)

project(Rasterizer)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(/usr/local/include)

file(GLOB ASSIGNMENT1_CPP_PATH ./*.cpp)
file(GLOB ASSIGNMENT1_H_PATH ./*.hpp)

add_executable(Rasterizer ${ASSIGNMENT1_CPP_PATH} ${ASSIGNMENT1_H_PATH})

target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
