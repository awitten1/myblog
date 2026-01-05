
cmake_minimum_required(VERSION 3.24)

project(BinSearch LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)


list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/install_dir")

find_package(benchmark REQUIRED)

add_library(functions functions.cpp)

add_executable(main main.cpp)

set(MARCH "" CACHE STRING "set march flag")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frecord-gcc-switches")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")

if (NOT MARCH STREQUAL "")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=${MARCH}")
endif()

target_link_libraries(main PRIVATE functions)
target_link_libraries(main PRIVATE benchmark::benchmark)
