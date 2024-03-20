cmake_minimum_required(VERSION 3.22)
project(
        boalang
        VERSION 0.1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;)

include(CTest)
add_subdirectory(src)
add_subdirectory(tests)
