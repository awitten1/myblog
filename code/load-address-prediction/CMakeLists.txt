cmake_minimum_required(VERSION 3.16)

project(load_address_prediction LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(benchmark CONFIG REQUIRED)

add_executable(load_address_prediction_bench experiments.cpp)
target_link_libraries(load_address_prediction_bench PRIVATE benchmark::benchmark)
