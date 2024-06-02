#include "interpreter_utils.hpp"

class InterpreterComparisonTests : public ::testing::TestWithParam<std::string> {};

TEST_P(InterpreterComparisonTests, comparison) {
  EXPECT_TRUE(str_contains(capture_interpreted_stdout("print " + GetParam() + ";"), "true"));
}

INSTANTIATE_TEST_SUITE_P(InterpreterGeneralTests, InterpreterComparisonTests,
    ::testing::Values(
        "1 == 1",
        "1.0 == 1.0",
        "true == true",
        "false == false",
        "true == !false",
        "\"abc\" == \"abc\"",
        "\"abc\" != \"def\"",
        "2 > 1",
        "2 >= 1",
        "2 >= 2",
        "1 < 2",
        "1 <= 2",
        "2 <= 2",
        "1.1 > 1.0",
        "1.1 >= 1.0",
        "1.1 >= 1.0",
        "\"b\" > \"a\"",
        "1 and 1.0",
        "\"Hello World!\" and 1.0",
        "true and true",
        "false or true",
        "true or false",
        "0 or true",
        "0.0 or true",
        "\"\" or true",
        "!0",
        "!0.0",
        "!\"\"",
        "!false"
    ));

class InterpreterIsTypeTests : public ::testing::TestWithParam<std::string> {};

TEST_P(InterpreterIsTypeTests, is_type) {
  EXPECT_TRUE(str_contains(capture_interpreted_stdout("print " + GetParam() + ";"), "true"));
}

INSTANTIATE_TEST_SUITE_P(InterpreterGeneralTests, InterpreterIsTypeTests,
    ::testing::Values(
    "1 is int",
    "!(1 is float)",
    "!(1 is bool)",
    "!(1 is str)",
    "1.0 is float",
    "!(1.0 is int)",
    "!(1.0 is bool)",
    "!(1.0 is str)",
    "true is bool",
    "false is bool",
    "!(true is int)",
    "!(true is float)",
    "!(true is str)",
    "\"Hello World!\" is str"
));
