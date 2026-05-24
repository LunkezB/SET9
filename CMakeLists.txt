cmake_minimum_required(VERSION 3.20)
project(string_sort_study LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(string_sort_study
    src/main.cpp
    src/StringGenerator.cpp
    src/StringAlgorithms.cpp
    src/StringSortTester.cpp
)

target_include_directories(string_sort_study PRIVATE include)

if (MSVC)
    target_compile_options(string_sort_study PRIVATE /W4 /permissive- /utf-8 /O2)
else()
    target_compile_options(string_sort_study PRIVATE -Wall -Wextra -Wpedantic -O3)
endif()
