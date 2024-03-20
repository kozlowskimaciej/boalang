if (BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    file(GLOB_RECURSE TEST_SOURCES "*.cpp")
    add_executable(
            boalang_tests
            ${TEST_SOURCES}
    )
    target_link_libraries(
            boalang_tests
            PRIVATE
            GTest::gtest_main
            boalang_lib
    )

    # allows using relative paths to /src in include directives
    target_include_directories(
            boalang_tests
            PUBLIC
            ${CMAKE_SOURCE_DIR}/src
    )

    include(GoogleTest)
    gtest_discover_tests(boalang_tests)
endif ()