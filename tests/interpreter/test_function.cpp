#include "interpreter_utils.hpp"

TEST(InterpreterFunctionTests, void_function) {
  std::string code = R"(
    void func() {
        print "func called";
    }
    func();
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "func called"));
}

TEST(InterpreterFunctionTests, void_function_return) {
  std::string code = R"(
    void func() {
        return;
    }
    func();
  )";
  capture_interpreted_stdout(code);
}

TEST(InterpreterFunctionTests, return_variant) {
  std::string code = R"(
    variant V {int};
    V func() {
        V v = 5;
        return v;
    }
    print func() as int;
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "5"));
}

TEST(InterpreterFunctionTests, return_struct) {
  std::string code = R"(
    struct S {
        int number;
    }

    S func() {
        S s2 = {5};
        return s2;
    }

    mut S s = {1};
    s = func();
    print s.number;
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "5"));
}

TEST(InterpreterFunctionTests, params) {
  std::string code = R"(
    void func(int c, str d) {
        print c as str;
        print d;
    }
    int a = 1;
    str b = "string";
    func(a, b);
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "string"));
}

TEST(InterpreterFunctionTests, mutate_args) {
  std::string code = R"(
    void func(str a) {
        a = "b";
        print "inner is: " + a;
    }
    str a = "a";
    print "outer was: " + a;
    func(a);
    print "outer is: " + a;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "outer was: a"));
  EXPECT_TRUE(str_contains(stdout, "inner is: b"));
  EXPECT_TRUE(str_contains(stdout, "outer is: a"));
}

TEST(InterpreterFunctionTests, recursion) {
  std::string code = R"(
    int fib(int n) {
        if (n == 1 or n == 2) {
          return 1;
        }
        return fib(n - 1) + fib(n - 2);
    }
    print fib(10);
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "55"));
}

TEST(InterpreterFunctionTests, call_context_redefinitions) {
  std::string code = R"(
    void func(int n) {
      variant V{int, float};
      void nested_func(){}
      int var = 1;
      if (n < 5) {
        n = n + 1;
        func(n);
      }
      return;
    }
    func(1);
  )";

  capture_interpreted_stdout(code);
}

TEST(InterpreterFunctionTests, max_recursion_depth_exceeded) {
  std::string code = R"(
    void func() {
      func();
    }
    func();
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Maximum recursion depth exceeded"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterFunctionTests, invalid_call_args_count) {
  std::string code = R"(
    void func(int a, int b) {}
    func(1);
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Invalid number of arguments"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterFunctionTests, call_args_type_mismatch) {
  std::string code = R"(
    void func(int a) {}
    func("string");
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Type mismatch in call arguments"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterFunctionTests, void_func_returns_value) {
  std::string code = R"(
    void func() {
      return 1;
    }
    func();
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Void function returned a value"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterFunctionTests, non_void_func_not_returns) {
  std::string code = R"(
    int func() {
      return;
    }
    func();
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(),
                                   "Non-void function did not return a value"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterFunctionTests, return_value_type_mismatch) {
  std::string code = R"(
    int func() {
      return "string";
    }
    func();
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(),
              "Function returned value with different type than declared"));
          throw;
        }
      },
      RuntimeError);
}
