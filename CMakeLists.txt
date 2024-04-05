cmake_minimum_required(VERSION 3.22)
project(
        boalang
        VERSION 0.1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
add_subdirectory(src)
add_subdirectory(tests)

file(GLOB_RECURSE ALL_CXX_SOURCE_FILES *.cpp *.hpp)

add_custom_target(
        clang-format
        COMMAND format
        -i
        -style=Google
        ${ALL_CXX_SOURCE_FILES}
)

add_custom_target(
        clang-tidy
        COMMAND clang-tidy
        -p .
        ${ALL_CXX_SOURCE_FILES}
)