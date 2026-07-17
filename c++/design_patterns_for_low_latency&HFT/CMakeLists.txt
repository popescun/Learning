cmake_minimum_required(VERSION 4.2.0)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(low_latency)

option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)

find_package(benchmark REQUIRED)

file(GLOB headers *.hpp)
add_executable(${PROJECT_NAME} main.cpp ${headers})
#include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} benchmark::benchmark)

if(ENABLE_TSAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread -g -O1)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
endif()