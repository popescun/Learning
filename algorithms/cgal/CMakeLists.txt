cmake_minimum_required(VERSION 3.30)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

project(cgal)

file(GLOB headers *.hpp)
add_executable(${PROJECT_NAME} main.cpp ${headers}
        sweep_line.hpp)
include_directories(${CMAKE_SOURCE_DIR}/../../../cgal/build/install/include)
target_link_libraries(${PROJECT_NAME} Boost::headers)