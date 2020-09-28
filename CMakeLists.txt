cmake_minimum_required(VERSION 3.8)
project(scheme LANGUAGES CXX)

# Sources for the executable are specified at end of CMakeLists.txt
add_executable(scheme "")

### Default build type

set(DEFAULT_CMAKE_BUILD_TYPE debug)

### Compile options

# Enable C++17 (Required)
set_property(TARGET scheme PROPERTY CXX_STANDARD 20)
set_property(TARGET scheme PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET scheme PROPERTY CXX_EXTENSIONS OFF)

# Common GCC/Clang(Linux) options
target_compile_options(scheme PRIVATE
                       -Wall
                       -g
                       -O0
                       )

#
#if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
#target_compile_options(9cc PRIVATE )
#endif()

#if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
#target_compile_options(9cc PRIVATE
#$<$<CONFIG:Debug>:-fno-limit-debug-info>)
#endif()

### Includes

target_include_directories(scheme PRIVATE src)

### Install

install(TARGETS scheme RUNTIME DESTINATION bin)

### Sources

target_sources(scheme PRIVATE
  src/main.cc
)
