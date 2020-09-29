cmake_minimum_required(VERSION 3.8)
project(scheme)

# Needed for tests
find_program(BASH_PROGRAM bash)

### Compile options

# Enable C++20 (Required)
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Add debug flags")
    set(_SCHEME_DEBUG_FLAGS
        -Wall -O0)
    set(_SCHEME_SANITIZERS
        -fsanitize=address -fsanitize=undefined)
    add_compile_options(${_SCHEME_DEBUG_FLAGS} ${_SCHEME_SANITIZERS})
    link_libraries(${_SCHEME_SANITIZERS})
  endif()
endif()

### Sources

add_subdirectory(src)
