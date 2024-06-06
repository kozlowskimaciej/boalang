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

TEST(InterpreterFunctionTests, pass_by_value_struct) {
  std::string code = R"(
    struct S {
        mut int number;
    }

    void func(S s) {
        s.number = 2;
    }

    mut S s = {1};
    func(s);
    print s.number;
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "1"));
}

TEST(InterpreterFunctionTests, pass_by_value_variant) {
  std::string code = R"(
    struct S {
        mut int number;
    }

    variant V {S};

    void func(V v2) {
      S s2 = {5};
      v2 = s2;
    }

    S s = {1};
    mut V v1 = s;
    func(v1);
    print (v1 as S).number;
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "1"));
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

TEST(InterpreterFunctionTests, param_redefinition) {
  std::string code = R"(
    int func(int a, int a) {
      return 1;
    }
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Param 'a' already defined in function"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterFunctionTests, return_stops_while) {
  std::string code = R"(
    void func(int a) {
        while(a>0) {
            print a;
            if (a == 6) {
                return;
            }
            a = a - 1;
        }
    }

    func(10);
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "10"));
  EXPECT_TRUE(str_contains(stdout, "9"));
  EXPECT_TRUE(str_contains(stdout, "8"));
  EXPECT_TRUE(str_contains(stdout, "7"));
  EXPECT_TRUE(str_contains(stdout, "6"));
  EXPECT_TRUE(!str_contains(stdout, "5"));
}
