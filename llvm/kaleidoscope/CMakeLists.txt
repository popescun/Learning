cmake_minimum_required(VERSION 4.1)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(toy)

find_package(benchmark REQUIRED)

file(GLOB headers *.hpp)
file(GLOB cpp_files *.cpp)
add_executable(${PROJECT_NAME}
  ${headers}
  ${cpp_files}
)

target_link_libraries(${PROJECT_NAME} benchmark::benchmark)