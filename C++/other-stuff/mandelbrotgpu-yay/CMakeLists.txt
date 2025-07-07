cmake_minimum_required(VERSION 3.10)
project(OpenCL_FP64_Test CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCL REQUIRED)

add_executable(mandelbrotgpu mandelbrotgpu.cpp)

target_include_directories(mandelbrotgpu PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(mandelbrotgpu PRIVATE ${OpenCL_LIBRARIES})
