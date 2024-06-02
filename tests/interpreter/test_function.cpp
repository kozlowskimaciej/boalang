#include "interpreter_utils.hpp"

TEST(InterpreterFunctionTests, void_function) {
  std::string code =
      R"V0G0N(
    void func() {
        print "func called";
    }
    func();
  )V0G0N";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "func called"));
}

TEST(InterpreterFunctionTests, params) {
  std::string code =
      R"V0G0N(
    void func(int c, str d) {
        print c as str;
        print d;
    }
    int a = 1;
    str b = "string";
    func(a, b);
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "string"));
}

TEST(InterpreterFunctionTests, mutate_args) {
  std::string code =
      R"V0G0N(
    void func(str a) {
        a = "b";
        print "inner is: " + a;
    }
    str a = "a";
    print "outer was: " + a;
    func(a);
    print "outer is: " + a;
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "outer was: a"));
  EXPECT_TRUE(str_contains(stdout, "inner is: b"));
  EXPECT_TRUE(str_contains(stdout, "outer is: a"));
}

TEST(InterpreterFunctionTests, recursion) {
  std::string code =
      R"V0G0N(
    int fib(int n) {
        if (n == 1 or n == 2) {
          return 1;
        }
        return fib(n - 1) + fib(n - 2);
    }
    print fib(10);
  )V0G0N";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "55"));
}
