#include "interpreter_utils.hpp"

TEST(InterpreterVariantTests, assigning) {
  std::string code = R"(
    variant V {int, float, str, bool};

    mut V v = "string";
    print v as str;

    v = 1;
    print v as int;

    v = 2.5;
    print v as float;

    v = true;
    print v as bool;

    v = false;
    print v as bool;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "string"));
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2.5"));
  EXPECT_TRUE(str_contains(stdout, "true"));
  EXPECT_TRUE(str_contains(stdout, "false"));
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

TEST(InterpreterVariantTests, nested_variant) {
  std::string code = R"(
    variant V1 {int};
    variant V2 {V1};
    V1 v1 = 1;
    V2 v2 = v1;
    print (v2 as V1) as int;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
}

TEST(InterpreterVariantTests, variant_assign_by_value) {
  std::string code = R"(
    variant V1 {int};
    variant V2 {V1, float};
    V1 v1 = 1;
    mut V2 v2 = v1;
    v2 = 2.0;  // assigning 2.0 to v2 should not affect v1 (1)
    print v1 as int;
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
}

TEST(InterpreterVariantTests, variant_assign_by_value_2) {
  std::string code = R"(
    variant V1 {int};
    variant V2 {V1};
    mut V1 v1 = 1;
    V2 v2 = v1;
    v1 = 2;  // assigning 2 to v1 should not affect v2 (1)
    print (v2 as V1) as int;
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

TEST(InterpreterVariantTests, inspect_complex_types) {
  std::string code = R"(
    struct S {
      int a;
    }
    variant V2 {int};
    variant V {V2, S};

    V2 v2 = 1;
    mut V v = v2;
    inspect v {
      V2 val => {print val as int;}
      default => {print false;}
    }

    S s = {2};
    v = s;
    inspect v {
      S val => {print val.a;}
      default => {print false;}
    }
  )";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2"));
  EXPECT_TRUE(!str_contains(stdout, "false"));
}