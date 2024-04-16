if (BUILD_TESTING)
    find_package(GTest REQUIRED)
    enable_testing()

    file(GLOB_RECURSE TEST_SOURCES "*.cpp")
    add_executable(
            boalang_tests
            ${TEST_SOURCES}
    )
    target_link_libraries(
            boalang_tests
            PRIVATE
            gtest::gtest
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