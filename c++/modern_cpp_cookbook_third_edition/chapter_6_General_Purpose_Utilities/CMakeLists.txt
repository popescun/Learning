cmake_minimum_required(VERSION 4.2.0)

# todo: compile errors with std::variant, switch to default apple clang
#set(CMAKE_C_COMPILER "/opt/homebrew/Cellar/llvm/21.1.6/bin/clang")
#set(CMAKE_CXX_COMPILER "/opt/homebrew/Cellar/llvm/21.1.6/bin/clang")

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(chapter_6)

file(GLOB headers *.hpp)
add_executable(${PROJECT_NAME} main.cpp ${headers})
#include_directories(${CMAKE_SOURCE_DIR}/include)