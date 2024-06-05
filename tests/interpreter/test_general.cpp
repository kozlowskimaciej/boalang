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
                      "1.5 >= 1.0", "\"b\" > \"a\"", "1 and 1.0",
                      "\"Hello World!\" and 1.0", "true and true",
                      "false or true", "true or false", "0 or true",
                      "0.0 or true", "\"\" or true", "!0", "!0.0", "!\"\"",
                      "!false"));

class InterpreterInvalidComparisonTests
    : public ::testing::TestWithParam<std::string> {};

TEST_P(InterpreterInvalidComparisonTests, invalid_comparison) {
  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout("print " + GetParam() + ";");
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(),
              "Comparison operation cannot be applied to different types"));
          throw;
        }
      },
      RuntimeError);
}

INSTANTIATE_TEST_SUITE_P(InterpreterGeneralTests,
                         InterpreterInvalidComparisonTests,
                         ::testing::Values("1 == 1.0", "1.0 == \"string\"",
                                           "true == \"true\"", "\"abc\" != 1",
                                           "2.0 > 1", "2.0 >= 1", "2.0 >= 2",
                                           "1.0 < 2"));

class InterpreterArithmeticTests
    : public ::testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(InterpreterArithmeticTests, arithmetic) {
  EXPECT_TRUE(str_contains(
      capture_interpreted_stdout("print " + GetParam().first + ";"),
      GetParam().second));
}

INSTANTIATE_TEST_SUITE_P(InterpreterGeneralTests, InterpreterArithmeticTests,
                         ::testing::Values(std::make_pair("1 + 1", "2"),
                                           std::make_pair("1.0 + 1.0", "2.0"),
                                           std::make_pair("\"a\" + \"b\"",
                                                          "ab"),
                                           std::make_pair("1 - 1", "0"),
                                           std::make_pair("2.0 - 1.0", "1.0")));

class InterpreterInvalidArithmeticTests
    : public ::testing::TestWithParam<std::string> {};

TEST_P(InterpreterInvalidArithmeticTests, invalid_arithmetic) {
  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout("print " + GetParam() + ";");
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(),
              "Arithmetic operation cannot be applied to different types"));
          throw;
        }
      },
      RuntimeError);
}

INSTANTIATE_TEST_SUITE_P(InterpreterGeneralTests,
                         InterpreterInvalidArithmeticTests,
                         ::testing::Values("1 + 1.0", "1 as float + 1",
                                           "\"abc\" - 5", "true + 1"));

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

TEST(InterpreterGeneralTests, negation) {
  std::string code = R"(
    print !true;
  )";
  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "false"));
}

TEST(InterpreterGeneralTests, addition) {
  std::string code = R"(
    print 3 + 3;
  )";
  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "6"));
}

TEST(InterpreterGeneralTests, subtraction) {
  std::string code = R"(
    print 3 - 3;
  )";
  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "0"));
}

TEST(InterpreterGeneralTests, multiplication) {
  std::string code = R"(
    print 3 * 3;
  )";
  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "9"));
}

TEST(InterpreterGeneralTests, division) {
  std::string code = R"(
    print 3 / 3;
  )";
  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "1"));
}

TEST(InterpreterGeneralTests, as_cast) {
  std::string code = R"(
    print 1 as float;
    print 2.4 as int;

    variant V {str};
    V v = "Hello World!";
    print v as str;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1.0"));
  EXPECT_TRUE(str_contains(stdout, "2"));
  EXPECT_TRUE(!str_contains(stdout, "2.4"));
  EXPECT_TRUE(str_contains(stdout, "Hello World!"));
}

TEST(InterpreterGeneralTests, as_bool_cast) {
  std::string code = R"(
    print true as bool;
    print 1 as bool;
    print 2.4 as bool;
    print "hi" as bool;

    variant V {str};
    V v = "Hello World!";
    print v as bool;

    struct S {mut int a;}
    S s = {1};
    print s as bool;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "true"));
  EXPECT_TRUE(!str_contains(stdout, "false"));
}

TEST(InterpreterGeneralTests, mutable_variables) {
  std::string code = R"(
    mut int a = 1;
    print a;
    a = 2;
    print a;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2"));
}

TEST(InterpreterGeneralTests, mutating_const) {
  std::string code = R"(
    int a = 1;
    print a;
    a = 2;
  )";

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
  std::string code = R"(
    int a = 1;
    int a = 2;
  )";

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
  std::string code = R"(
    void a(){}
    int a(){}
  )";

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
  std::string code = R"(
    variant a{int};
    variant a{float};
  )";

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
  std::string code = R"(
    mut int a = 1;
    while (a <= 5) {
        print a;
        a = a + 1;
    }
  )";

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
  std::string code = R"(
    if (true) {
      print 0;
    }
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "0"));
}

TEST(InterpreterGeneralTests, else_statement) {
  std::string code = R"(
    if (false) {
      print 0;
    } else {
      print 1;
    }
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
}

TEST(InterpreterGeneralTests, variable_block_redefinition) {
  std::string code = R"(
    int a = 5;
    {
        int a = 10;
    }
  )";

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

TEST(InterpreterGeneralTests, variable_undefined) {
  std::string code = R"(
    print a;
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Identifier 'a' not defined"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterGeneralTests, type_undefined) {
  std::string code = R"(
    A a = 1;
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Type 'A' not defined"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterGeneralTests, function_undefined) {
  std::string code = R"(
    A();
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Function 'A' not defined"));
          throw;
        }
      },
      RuntimeError);
}
