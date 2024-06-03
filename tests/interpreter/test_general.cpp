#include <iostream>

#include "interpreter_utils.hpp"

class InterpreterComparisonTests
    : public ::testing::TestWithParam<std::string> {};

TEST_P(InterpreterComparisonTests, comparison) {
  EXPECT_TRUE(str_contains(
      capture_interpreted_stdout("print " + GetParam() + ";"), "true"));
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterGeneralTests, InterpreterComparisonTests,
    ::testing::Values("1 == 1", "1.0 == 1.0", "true == true", "false == false",
                      "true == !false", "\"abc\" == \"abc\"",
                      "\"abc\" != \"def\"", "2 > 1", "2 >= 1", "2 >= 2",
                      "1 < 2", "1 <= 2", "2 <= 2", "1.1 > 1.0", "1.1 >= 1.0",
                      "1.1 >= 1.0", "\"b\" > \"a\"", "1 and 1.0",
                      "\"Hello World!\" and 1.0", "true and true",
                      "false or true", "true or false", "0 or true",
                      "0.0 or true", "\"\" or true", "!0", "!0.0", "!\"\"",
                      "!false"));

class InterpreterIsTypeTests : public ::testing::TestWithParam<std::string> {};

TEST_P(InterpreterIsTypeTests, is_type) {
  EXPECT_TRUE(str_contains(
      capture_interpreted_stdout("print " + GetParam() + ";"), "true"));
}

INSTANTIATE_TEST_SUITE_P(
    InterpreterGeneralTests, InterpreterIsTypeTests,
    ::testing::Values("1 is int", "!(1 is float)", "!(1 is bool)",
                      "!(1 is str)", "1.0 is float", "!(1.0 is int)",
                      "!(1.0 is bool)", "!(1.0 is str)", "true is bool",
                      "false is bool", "!(true is int)", "!(true is float)",
                      "!(true is str)", "\"Hello World!\" is str"));

TEST(InterpreterGeneralTests, mutable_variables) {
  std::string code = R"V0G0N(
    mut int a = 1;
    print a;
    a = 2;
    print a;
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2"));
}

TEST(InterpreterGeneralTests, mutating_const) {
  std::string code = R"V0G0N(
    int a = 1;
    print a;
    a = 2;
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Tried assigning value to a const"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterGeneralTests, variable_redefinition) {
  std::string code = R"V0G0N(
    int a = 1;
    int a = 2;
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Identifier 'a' already defined"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterGeneralTests, function_redefinition) {
  std::string code = R"V0G0N(
    void a(){}
    int a(){}
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Function 'a' already defined"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterGeneralTests, type_redefinition) {
  std::string code = R"V0G0N(
    variant a{int};
    variant a{float};
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Type 'a' already defined"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterGeneralTests, while_loop) {
  std::string code = R"V0G0N(
    mut int a = 1;
    while (a <= 5) {
        print a;
        a = a + 1;
    }
  )V0G0N";

  std::cerr << "1";
  auto stdout = capture_interpreted_stdout(code);
  std::cerr << stdout;
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2"));
  EXPECT_TRUE(str_contains(stdout, "3"));
  EXPECT_TRUE(str_contains(stdout, "4"));
  EXPECT_TRUE(str_contains(stdout, "5"));
}

TEST(InterpreterGeneralTests, if_statement) {
  std::string code = R"V0G0N(
    if (true) {
      print 0;
    }
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "0"));
}

TEST(InterpreterGeneralTests, else_statement) {
  std::string code = R"V0G0N(
    if (false) {
      print 0;
    } else {
      print 1;
    }
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
}

TEST(InterpreterGeneralTests, block_scope) {
  std::string code = R"V0G0N(
    int a = 5;
    print "outer was: " + a as str;
    {
        int a = 10;
        print "inner is: " + a as str;
    }
    print "outer is: " + a as str;
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "outer was: 5"));
  EXPECT_TRUE(str_contains(stdout, "inner is: 10"));
  EXPECT_TRUE(str_contains(stdout, "outer is: 5"));
}
