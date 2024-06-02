#include "interpreter_utils.hpp"

TEST(InterpreterStructTests, mutate_field) {
  std::string code = R"V0G0N(
    struct S {mut int a; float b;}
    S st_obj = {6, 1.0};

    print st_obj.a;
    st_obj.a = 3;
    print st_obj.a;

    print st_obj.b;
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "6"));
  EXPECT_TRUE(str_contains(stdout, "3"));
  EXPECT_TRUE(str_contains(stdout, "1.0"));
}

// TEST(InterpreterStructTests, struct_assign) {
// std::string code = R"V0G0N(
//     struct S {mut int a;}
//     mut S s = {1};
//     S d = {2};
//     s = d;
//     s.a=3;
//     print d.a;
//     print s.a;
//   )V0G0N";
//
// auto stdout = capture_interpreted_stdout(code);
// EXPECT_TRUE(str_contains(stdout, "2"));
// EXPECT_TRUE(str_contains(stdout, "3"));
// }

TEST(InterpreterStructTests, nested_struct) {
  std::string code = R"V0G0N(
    struct A {
      mut int number;
    }

    struct S {
      mut A nested;
    }

    A b = {1};
    S a = {b};

    print a.nested.number;
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
}

TEST(InterpreterStructTests, variant_in_struct) {
  std::string code = R"V0G0N(
    variant V {int, float};
    struct S {
      mut V number;
    }

    S a = {1};
    print a.number as int;

    a.number=2.0;
    print a.number as float;
  )V0G0N";

  auto stdout = capture_interpreted_stdout(code);
  EXPECT_TRUE(str_contains(stdout, "1"));
  EXPECT_TRUE(str_contains(stdout, "2.0"));
}

TEST(InterpreterStructTests, init_list_type_mismatch) {
  std::string code = R"V0G0N(
    struct A {
      mut int number;
    }
    A a = {1.0};
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(
              e.what(), "Type mismatch in initalizer list for 'a.number'"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterStructTests, no_init_list) {
  std::string code = R"V0G0N(
    struct A {
      mut int number;
    }
    A a = 1;
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Expected initalizer list"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterStructTests, init_list_invalid) {
  std::string code = R"V0G0N(
    struct A {
      mut int number;
    }
    A a = {1, 2};
  )V0G0N";

  EXPECT_THROW(
      {
        try {
          capture_interpreted_stdout(code);
        } catch (const RuntimeError& e) {
          EXPECT_TRUE(str_contains(e.what(),
                                   "Different number of struct fields and "
                                   "values in initalizer list"));
          throw;
        }
      },
      RuntimeError);
}

TEST(InterpreterStructTests, const_field_assign) {
  std::string code = R"V0G0N(
    struct A {
      int number;
    }
    A a = {1};
    a.number = 2;
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

TEST(InterpreterStructTests, field_assign_type_mismatch) {
  std::string code = R"V0G0N(
    struct A {
      mut int number;
    }
    A a = {1};
    a.number = 2.0;
  )V0G0N";

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