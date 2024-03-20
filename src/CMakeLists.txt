file(GLOB SOURCE_FILES source/*.cpp source/*.hpp)
file(GLOB LEXER_FILES lexer/*.cpp lexer/*.hpp)

add_library(
        boalang_lib
        main.cpp
        ${SOURCE_FILES}
        ${LEXER_FILES}
)
add_executable(
        boalang
        main.cpp
)
target_link_libraries(
        boalang
        boalang_lib
)

set(COMMON_FLAGS -Wall -Wfloat-conversion -Wextra -pedantic -Werror)
target_compile_options(boalang PUBLIC ${COMMON_FLAGS})
target_compile_options(boalang_lib PUBLIC ${COMMON_FLAGS})
target_include_directories(boalang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})