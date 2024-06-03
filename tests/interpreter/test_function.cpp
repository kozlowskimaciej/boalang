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
