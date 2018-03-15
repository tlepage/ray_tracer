cmake_minimum_required(VERSION 3.9)
project(raytracer)
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2")
set(SOURCE_FILES src/RayTracer.cpp include/RayTracer.h src/Bitmap.cpp include/Bitmap.h include/Math.h tests/math_test.cpp include/Vector.h)
add_executable(raytracer ${SOURCE_FILES})

target_link_libraries(raytracer gtest gtest_main)