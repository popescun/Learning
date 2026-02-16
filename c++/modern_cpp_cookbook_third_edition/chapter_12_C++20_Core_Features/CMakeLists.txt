cmake_minimum_required(VERSION 4.2.0)

set(CMAKE_C_COMPILER "/opt/homebrew/Cellar/llvm/21.1.8/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/Cellar/llvm/21.1.8/bin/clang++")

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(chapter_12)

# ASAN
# this should be added before target is added
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

file(GLOB headers *.hpp)
add_executable(${PROJECT_NAME} main.cpp ${headers})
#include_directories(${CMAKE_SOURCE_DIR}/include)

# ASAN
# this should be added after target is added
target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)

# TSAN
#target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=thread)
#target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=thread)
