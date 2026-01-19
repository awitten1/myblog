
cmake_minimum_required(VERSION 3.24)

project(BinSearch LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTS "Build unit tests" ON)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/install_dir")

find_package(benchmark REQUIRED)

add_library(functions functions.cpp)
target_include_directories(functions INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_executable(main main.cpp)

set(MARCH "" CACHE STRING "set march flag")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frecord-gcc-switches")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")

if (NOT MARCH STREQUAL "")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=${MARCH}")
endif()

target_link_libraries(main PRIVATE functions)
target_link_libraries(main PRIVATE benchmark::benchmark)

if (BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_executable(test_binary_search tests/test_binary_search.cpp)
    target_link_libraries(test_binary_search PRIVATE functions GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_binary_search)
endif()
