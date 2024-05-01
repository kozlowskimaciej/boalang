set(CMAKE_CXX_CLANG_TIDY clang-tidy;)

file(GLOB TOKEN_FILES token/*.cpp token/*.hpp)
file(GLOB SOURCE_FILES source/*.cpp source/*.hpp)
file(GLOB LEXER_FILES lexer/*.cpp lexer/*.hpp)
file(GLOB EXPR_FILES expr/*.cpp expr/*.hpp)
file(GLOB STMT_FILES stmt/*.cpp stmt/*.hpp)
file(GLOB PARSER_FILES parser/*.cpp parser/*.hpp)

find_package(magic_enum REQUIRED)
find_package(argparse REQUIRED)

add_library(
        boalang_lib
        STATIC
        ${TOKEN_FILES}
        ${SOURCE_FILES}
        ${LEXER_FILES}
        ${EXPR_FILES}
        ${STMT_FILES}
        ${PARSER_FILES}
)
target_link_libraries(
        boalang_lib
        PRIVATE
        magic_enum::magic_enum
)

add_executable(
        boalang
        main.cpp
)
target_link_libraries(
        boalang
        PRIVATE
        boalang_lib
        argparse::argparse
        -fsanitize=undefined
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Using Clang")
    list(APPEND COMMON_FLAGS -fsanitize=undefined)
    target_link_options(boalang PUBLIC -fsanitize=undefined)
endif()

set(COMMON_FLAGS -Wall -Wfloat-conversion -Wextra -pedantic -Werror)
target_compile_options(boalang PUBLIC ${COMMON_FLAGS})
target_compile_options(boalang_lib PUBLIC ${COMMON_FLAGS})
target_include_directories(boalang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
