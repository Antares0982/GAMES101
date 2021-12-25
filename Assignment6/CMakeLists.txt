cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

file(GLOB ASSIGNMENT_CPP_PATH ./*.cpp)
file(GLOB ASSIGNMENT_H_PATH ./*.hpp)

add_executable(RayTracing ${ASSIGNMENT_CPP_PATH} ${ASSIGNMENT_H_PATH})

#add_executable(RayTracing main.cpp Object.hpp Vector.cpp Vector.hpp Sphere.hpp global.hpp Triangle.hpp Scene.cpp
#        Scene.hpp Light.hpp AreaLight.hpp BVH.cpp BVH.hpp Bounds3.hpp Ray.hpp Material.hpp Intersection.hpp
#        Renderer.cpp Renderer.hpp)
