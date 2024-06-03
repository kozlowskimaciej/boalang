#include "interpreter_utils.hpp"

TEST(InterpreterVariantTests, assigning) {
  std::string code = R"(
    variant V {int, float, str};

    mut V v = "string";
    print v as str;

    v = 1;
    print v as int;

    v = 2.5;
    print v as float;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "string"));
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2.5"));
}

TEST(InterpreterVariantTests, struct_in_variant) {
  std::string code = R"(
    struct S {int a;}
    variant V {S};
    S s = {1};
    V v = s;
    print (v as S).a;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
}

TEST(InterpreterVariantTests, invalid_variant_cast) {
  std::string code = R"(
    variant V {int, float, str};

    V v = "string";
    print v as int;
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Invalid contained value type cast"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterVariantTests, invalid_variant_initalization) {
  std::string code = R"(
    variant V {int};

    V v = "string";
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(),
              "Tried to initialize 'v' with value of different type"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterVariantTests, invalid_variant_assign) {
  std::string code = R"(
    variant V {int};

    mut V v = 1;
    v = "string";
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(), "Tried assigning value with different type"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterVariantTests, variant_const_assign) {
  std::string code = R"(
    variant V {int};

    V v = 1;
    v = 2;
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

TEST(InterpreterVariantTests, inspect) {
  std::string code = R"(
    variant V {int, float};
    V v = 1.0;
    inspect v {
      int val => {print "I'm an integer";}
      float val => {print "I'm a float";}
    }
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "I'm a float"));
}

TEST(InterpreterVariantTests, inspect_default) {
  std::string code = R"(
    variant V {int, float, str};
    V v = "just a string";
    inspect v {
      int val => {print "I'm an integer";}
      float val => {print "I'm a float";}
      default => {print "Who am I?";}
    }
  )";

  EXPECT_TRUE(str_contains(capture_interpreted_stdout(code), "Who am I?"));
}

TEST(InterpreterVariantTests, inspect_nonexhaustive) {
  std::string code = R"(
    variant V {int, float, str};
    V v = "just a string";
    inspect v {
      int val => {print "I'm an integer";}
      float val => {print "I'm a float";}
    }
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(),
              "Inspect did not match any types and default not present"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterVariantTests, inspect_non_variant) {
  std::string code = R"(
    int a = 1;
    inspect a {
      int val => {print "I'm an integer";}
      float val => {print "I'm a float";}
    }
  )";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Cannot inspect non-variant objects"));
          throw;
        }
      },
      RuntimeError);
}
