cmake_minimum_required(VERSION 3.30)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(leetcode)

file(GLOB headers *.hpp)
add_executable(${PROJECT_NAME} main.cpp ${headers})
#include_directories(${CMAKE_SOURCE_DIR}/include)