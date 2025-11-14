cmake_minimum_required(VERSION 4.1)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(toy)

# https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
set(LLVM_PATH /Users/popescun/github/llvm-project/llvm/install)
link_directories(${LLVM_PATH}/lib)
include_directories(${LLVM_PATH}/include)
# add cmake module for function `llvm_map_components_to_libnames` that generates llvm libs list
include(${LLVM_PATH}/lib/cmake/llvm/LLVM-Config.cmake)
llvm_map_components_to_libnames(llvm_libs
  support
  core
  demangle
  remarks
  bitstreamreader
  binaryformat
  targetparser
)

#find_package(LLVM REQUIRED CONFIG)
find_package(benchmark REQUIRED)

file(GLOB headers *.hpp)
file(GLOB cpp_files *.cpp)
add_executable(${PROJECT_NAME}
  ${headers}
  ${cpp_files}
)

target_link_libraries(${PROJECT_NAME}
  ${llvm_libs}
  benchmark::benchmark
)