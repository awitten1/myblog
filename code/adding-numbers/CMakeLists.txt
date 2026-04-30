cmake_minimum_required(VERSION 3.14)
project(adding_numbers LANGUAGES CXX ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(benchmark REQUIRED)

add_executable(bench_add bench_add.cpp add_integers.asm)
target_link_libraries(bench_add PRIVATE benchmark::benchmark)
