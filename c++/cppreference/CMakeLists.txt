cmake_minimum_required(VERSION 4.2.0)

# C++20 modules need clang-scan-deps, which Apple Clang does not ship, so use
# Homebrew LLVM. Changing the compiler requires a fresh build directory.
set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(cppreference)

file(GLOB headers *.hpp cpp20/*.hpp)
add_executable(${PROJECT_NAME} main.cpp ${headers})
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES cpp20/mylib.cpp)
include_directories(${CMAKE_SOURCE_DIR}/cpp20)


# Sanitizer selection. ASan and TSan cannot be combined in one build, so pick
# one: cmake -DSANITIZER=asan (default, detects memory errors) or
#       cmake -DSANITIZER=tsan (detects data races, e.g. memory_model_order ex.1)
set(SANITIZER "asan" CACHE STRING "Sanitizer to use: asan, tsan, or none")
set_property(CACHE SANITIZER PROPERTY STRINGS asan tsan none)

if(SANITIZER STREQUAL "asan")
  set(SANITIZE_FLAG -fsanitize=address)
elseif(SANITIZER STREQUAL "tsan")
  set(SANITIZE_FLAG -fsanitize=thread)
elseif(SANITIZER STREQUAL "none")
  set(SANITIZE_FLAG "")
else()
  message(FATAL_ERROR "Unknown SANITIZER '${SANITIZER}' (use asan, tsan, or none)")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${SANITIZE_FLAG} -fno-omit-frame-pointer -g)
target_link_options(${PROJECT_NAME} PRIVATE ${SANITIZE_FLAG})