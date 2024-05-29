#include <gtest/gtest.h>

#include "../utils.hpp"
#include "parser/parser.hpp"

TEST(ParserErrorTest, missing_semicolon) {
  StringSource source("if (true) print \"true\" else print \"false\";");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Expected ';' after printed expression."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_ELSE);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserErrorTest, missing_semicolon_2) {
  StringSource source("a = 2");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Expected ';' after assignment."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_ETX);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserErrorTest, missing_semicolon_etx) {
  StringSource source("print \"Hello World\"");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Expected ';' after printed expression."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_ETX);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserErrorTest, comparison_instead_of_assignment) {
  StringSource source("a == 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(str_contains(
              e.what(), "Expected assignment, call or declaration."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_EQUAL_EQUAL);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserErrorTest, invalid_func_call) {
  StringSource source("a(1, 2 3);");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Excepted ')' after call arguments."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_INT_VAL);
          EXPECT_EQ(e.get_token().stringify(), "3");
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserErrorTest, if_condition_not_closed) {
  StringSource source("if(true print true;");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Expected ')' after condition."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_PRINT);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserErrorTest, block_stmt_not_closed) {
  StringSource source("{ print true;");
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(
              str_contains(e.what(), "Expected '}' after block statement."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_ETX);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserTest, print_primary_str) {
  StringSource source("print \"Hello World\";");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto expr = dynamic_cast<LiteralExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(std::get<std::string>(expr->literal), "Hello World");
}

TEST(ParserTest, print_primary_int_val) {
  StringSource source("print 1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto expr = dynamic_cast<LiteralExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(std::get<int>(expr->literal), 1);
}

TEST(ParserTest, print_primary_float_val) {
  StringSource source("print 1.0;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto expr = dynamic_cast<LiteralExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(std::get<float>(expr->literal), 1.f);
}

TEST(ParserTest, print_primary_bool) {
  StringSource source("print true;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto expr = dynamic_cast<LiteralExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(std::get<bool>(expr->literal), true);
}

TEST(ParserTest, print_primary_identifier) {
  StringSource source("print id;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto expr = dynamic_cast<VarExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(expr->identifier, "id");
}

TEST(ParserTest, print_primary_grouping) {
  StringSource source("print (1);");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());

  EXPECT_TRUE(print_stmt != nullptr);
  auto grouping_expr = dynamic_cast<GroupingExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(grouping_expr != nullptr);

  auto expr = dynamic_cast<LiteralExpr*>(grouping_expr->expr.get());
  EXPECT_EQ(std::get<int>(expr->literal), 1);
}

TEST(ParserTest, print_primary_initalizer) {
  StringSource source("print {1, 2};");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto initalizerlist_expr =
      dynamic_cast<InitalizerListExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(initalizerlist_expr != nullptr);
  EXPECT_EQ(initalizerlist_expr->list.size(), 2);

  auto expr = dynamic_cast<LiteralExpr*>(initalizerlist_expr->list[0].get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(std::get<int>(expr->literal), 1);

  expr = dynamic_cast<LiteralExpr*>(initalizerlist_expr->list[1].get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(std::get<int>(expr->literal), 2);
}

TEST(ParserTest, print_call_function_no_args) {
  StringSource source("print foo();");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto call_expr = dynamic_cast<CallExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(call_expr != nullptr);
  EXPECT_EQ(call_expr->arguments.size(), 0);

  auto callee = dynamic_cast<VarExpr*>(call_expr->callee.get());
  EXPECT_TRUE(callee != nullptr);
  EXPECT_EQ(callee->identifier, "foo");
}

TEST(ParserTest, print_call_function_args) {
  StringSource source("print foo(1, 2);");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto call_expr = dynamic_cast<CallExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(call_expr != nullptr);
  EXPECT_EQ(call_expr->arguments.size(), 2);

  auto arg = dynamic_cast<LiteralExpr*>(call_expr->arguments[0].get());
  EXPECT_TRUE(arg != nullptr);
  EXPECT_EQ(std::get<int>(arg->literal), 1);

  arg = dynamic_cast<LiteralExpr*>(call_expr->arguments[1].get());
  EXPECT_TRUE(arg != nullptr);
  EXPECT_EQ(std::get<int>(arg->literal), 2);

  auto callee = dynamic_cast<VarExpr*>(call_expr->callee.get());
  EXPECT_TRUE(callee != nullptr);
  EXPECT_EQ(callee->identifier, "foo");
}

TEST(ParserTest, print_call_function_args_over_limit) {
  std::string code = "print foo(";
  for (size_t i = 0; i <= 257; ++i) {
    code += "1, ";
  }
  code += "1);";
  StringSource source(code);
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW(
      {
        try {
          parser.parse();
        } catch (const SyntaxError& e) {
          EXPECT_TRUE(str_contains(
              e.what(), "Maximum amount (256) of arguments exceeded."));
          EXPECT_EQ(e.get_token().get_type(), TokenType::TOKEN_COMMA);
          throw;
        }
      },
      SyntaxError);
}

TEST(ParserTest, print_call_field_access) {
  StringSource source("print a.b;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto fieldaccess_expr =
      dynamic_cast<FieldAccessExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(fieldaccess_expr != nullptr);
  EXPECT_EQ(fieldaccess_expr->field_name, "b");

  auto parent_struct =
      dynamic_cast<VarExpr*>(fieldaccess_expr->parent_struct.get());
  EXPECT_TRUE(parent_struct != nullptr);
  EXPECT_EQ(parent_struct->identifier, "a");
}

TEST(ParserTypeCastExprTest, print_type_as) {
  StringSource source("print a as int;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto cast_expr = dynamic_cast<AsTypeExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(cast_expr != nullptr);
  EXPECT_EQ(cast_expr->type.type, INT);

  auto left = dynamic_cast<VarExpr*>(cast_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->identifier, "a");
}

TEST(ParserTypeCastExprTest, print_type_is) {
  StringSource source("print a is int;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto cast_expr = dynamic_cast<IsTypeExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(cast_expr != nullptr);
  EXPECT_EQ(cast_expr->type.type, INT);

  auto left = dynamic_cast<VarExpr*>(cast_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->identifier, "a");
}

TEST(ParserUnaryExprTest, print_negation) {
  StringSource source("print -a;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto unary_expr = dynamic_cast<NegationExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(unary_expr != nullptr);

  auto left = dynamic_cast<VarExpr*>(unary_expr->right.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->identifier, "a");
}

TEST(ParserUnaryExprTest, print_logicalnegation) {
  StringSource source("print !a;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto unary_expr = dynamic_cast<LogicalNegationExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(unary_expr != nullptr);

  auto left = dynamic_cast<VarExpr*>(unary_expr->right.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->identifier, "a");
}

TEST(ParserBinaryExprTest, print_binary) {
  StringSource source("print 1 + 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<AdditionExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);

  auto left = dynamic_cast<LiteralExpr*>(binary_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(std::get<int>(left->literal), 1);

  auto right = dynamic_cast<LiteralExpr*>(binary_expr->right.get());
  EXPECT_TRUE(right != nullptr);
  EXPECT_EQ(std::get<int>(right->literal), 2);
}

TEST(ParserBinaryExprTest, print_notequalcomp) {
  StringSource source("print 1 != 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<NotEqualCompExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_equalcomp) {
  StringSource source("print 1 == 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<EqualCompExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_greatercomp) {
  StringSource source("print 1 > 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<GreaterCompExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_greaterequalcomp) {
  StringSource source("print 1 >= 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr =
      dynamic_cast<GreaterEqualCompExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_lesscomp) {
  StringSource source("print 1 < 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<LessCompExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_lessequalcomp) {
  StringSource source("print 1 <= 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<LessEqualCompExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_subtraction) {
  StringSource source("print 1 - 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<SubtractionExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_addition) {
  StringSource source("print 1 + 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<AdditionExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_division) {
  StringSource source("print 1 / 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<DivisionExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserBinaryExprTest, print_multiplication) {
  StringSource source("print 1 * 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<MultiplicationExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
}

TEST(ParserLogicalExprTest, print_and) {
  StringSource source("print 1 and 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto logical_expr = dynamic_cast<LogicalAndExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(logical_expr != nullptr);

  auto left = dynamic_cast<LiteralExpr*>(logical_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(std::get<int>(left->literal), 1);

  auto right = dynamic_cast<LiteralExpr*>(logical_expr->right.get());
  EXPECT_TRUE(right != nullptr);
  EXPECT_EQ(std::get<int>(right->literal), 2);
}

TEST(ParserLogicalExprTest, print_or) {
  StringSource source("print 1 or 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto logical_expr = dynamic_cast<LogicalOrExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(logical_expr != nullptr);
}

TEST(ParserTest, block_stmt) {
  StringSource source("{print \"Hello World\";}");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto block_stmt = dynamic_cast<BlockStmt*>(program->statements[0].get());
  EXPECT_TRUE(block_stmt != nullptr);

  EXPECT_EQ(block_stmt->statements.size(), 1);
  auto print_stmt = dynamic_cast<PrintStmt*>(block_stmt->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);
}

TEST(ParserTest, inspect_stmt) {
  StringSource source(
      "inspect variant_obj {"
      "int val => print val;"
      "float val => print val;"
      "default => print \"default\";"
      "}");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto inspect_stmt = dynamic_cast<InspectStmt*>(program->statements[0].get());
  EXPECT_TRUE(inspect_stmt != nullptr);

  auto inspected = dynamic_cast<VarExpr*>(inspect_stmt->inspected.get());
  EXPECT_TRUE(inspected != nullptr);
  EXPECT_EQ(inspected->identifier, "variant_obj");

  EXPECT_EQ(inspect_stmt->lambdas.size(), 2);
  auto lambda_stmt =
      dynamic_cast<LambdaFuncStmt*>(inspect_stmt->lambdas[0].get());
  EXPECT_TRUE(lambda_stmt != nullptr);
  EXPECT_EQ(lambda_stmt->type.type, INT);

  EXPECT_EQ(lambda_stmt->identifier, "val");
  EXPECT_TRUE(dynamic_cast<PrintStmt*>(lambda_stmt->body.get()) != nullptr);

  lambda_stmt = dynamic_cast<LambdaFuncStmt*>(inspect_stmt->lambdas[1].get());
  EXPECT_TRUE(lambda_stmt != nullptr);
  EXPECT_EQ(lambda_stmt->type.type, FLOAT);

  EXPECT_EQ(lambda_stmt->identifier, "val");
  EXPECT_TRUE(dynamic_cast<PrintStmt*>(lambda_stmt->body.get()) != nullptr);

  EXPECT_TRUE(dynamic_cast<PrintStmt*>(inspect_stmt->default_lambda.get()) !=
              nullptr);
}

TEST(ParserTest, return_stmt) {
  StringSource source("return 1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto return_stmt = dynamic_cast<ReturnStmt*>(program->statements[0].get());
  EXPECT_TRUE(return_stmt != nullptr);

  auto value = dynamic_cast<LiteralExpr*>(return_stmt->value.get());
  EXPECT_TRUE(value != nullptr);
  EXPECT_EQ(std::get<int>(value->literal), 1);
}

TEST(ParserTest, return_stmt_void) {
  StringSource source("return;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto return_stmt = dynamic_cast<ReturnStmt*>(program->statements[0].get());
  EXPECT_TRUE(return_stmt != nullptr);
  EXPECT_TRUE(return_stmt->value == nullptr);
}

TEST(ParserTest, while_stmt) {
  StringSource source("while(true) print 1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto while_stmt = dynamic_cast<WhileStmt*>(program->statements[0].get());
  EXPECT_TRUE(while_stmt != nullptr);

  auto condition = dynamic_cast<LiteralExpr*>(while_stmt->condition.get());
  EXPECT_TRUE(condition != nullptr);
  EXPECT_EQ(std::get<bool>(condition->literal), true);

  EXPECT_TRUE(dynamic_cast<PrintStmt*>(while_stmt->body.get()) != nullptr);
}

TEST(ParserTest, if_stmt) {
  StringSource source("if(true) print 1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto if_stmt = dynamic_cast<IfStmt*>(program->statements[0].get());
  EXPECT_TRUE(if_stmt != nullptr);

  auto condition = dynamic_cast<LiteralExpr*>(if_stmt->condition.get());
  EXPECT_TRUE(condition != nullptr);
  EXPECT_EQ(std::get<bool>(condition->literal), true);

  EXPECT_TRUE(dynamic_cast<PrintStmt*>(if_stmt->then_branch.get()) != nullptr);
  EXPECT_TRUE(if_stmt->else_branch == nullptr);
}

TEST(ParserTest, if_else_stmt) {
  StringSource source("if(true) print 1; else return 1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto if_stmt = dynamic_cast<IfStmt*>(program->statements[0].get());
  EXPECT_TRUE(if_stmt != nullptr);

  auto condition = dynamic_cast<LiteralExpr*>(if_stmt->condition.get());
  EXPECT_TRUE(condition != nullptr);
  EXPECT_EQ(std::get<bool>(condition->literal), true);

  EXPECT_TRUE(dynamic_cast<PrintStmt*>(if_stmt->then_branch.get()) != nullptr);
  EXPECT_TRUE(dynamic_cast<ReturnStmt*>(if_stmt->else_branch.get()) != nullptr);
}

TEST(ParserTest, struct_decl_stmt) {
  StringSource source(
      "struct S {"
      "    mut int a;"
      "    float b;"
      "}");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto struct_stmt =
      dynamic_cast<StructDeclStmt*>(program->statements[0].get());
  EXPECT_TRUE(struct_stmt != nullptr);
  EXPECT_EQ(struct_stmt->identifier, "S");

  EXPECT_EQ(struct_stmt->fields.size(), 2);
  auto field = dynamic_cast<StructFieldStmt*>(struct_stmt->fields[0].get());
  EXPECT_TRUE(field != nullptr);
  EXPECT_EQ(field->mut, true);
  EXPECT_EQ(field->type.type, INT);

  EXPECT_EQ(field->identifier, "a");

  field = dynamic_cast<StructFieldStmt*>(struct_stmt->fields[1].get());
  EXPECT_TRUE(field != nullptr);
  EXPECT_EQ(field->mut, false);
  EXPECT_EQ(field->type.type, FLOAT);

  EXPECT_EQ(field->identifier, "b");
}

TEST(ParserTest, variant_decl_stmt) {
  StringSource source("variant V { int, float, S };");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto variant_stmt =
      dynamic_cast<VariantDeclStmt*>(program->statements[0].get());
  EXPECT_TRUE(variant_stmt != nullptr);
  EXPECT_EQ(variant_stmt->identifier, "V");

  EXPECT_EQ(variant_stmt->params.size(), 3);
  EXPECT_EQ(variant_stmt->params[0].type, INT);
  EXPECT_EQ(variant_stmt->params[1].type, FLOAT);
  EXPECT_EQ(variant_stmt->params[2].name, "S");
}

TEST(ParserTest, assign_stmt) {
  StringSource source("a = 1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto assign_stmt = dynamic_cast<AssignStmt*>(program->statements[0].get());
  EXPECT_TRUE(assign_stmt != nullptr);

  auto var = dynamic_cast<VarExpr*>(assign_stmt->var.get());
  EXPECT_TRUE(var != nullptr);
  EXPECT_EQ(var->identifier, "a");

  EXPECT_TRUE(dynamic_cast<LiteralExpr*>(assign_stmt->value.get()) != nullptr);
}

TEST(ParserTest, call_stmt) {
  StringSource source("a(1);");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto call_stmt = dynamic_cast<CallStmt*>(program->statements[0].get());
  EXPECT_TRUE(call_stmt != nullptr);
  EXPECT_EQ(call_stmt->identifier, "a");
  EXPECT_EQ(call_stmt->arguments.size(), 1);
  EXPECT_TRUE(dynamic_cast<LiteralExpr*>(call_stmt->arguments[0].get()) !=
              nullptr);
}

TEST(ParserTest, call_stmt_no_args) {
  StringSource source("a();");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto call_stmt = dynamic_cast<CallStmt*>(program->statements[0].get());
  EXPECT_TRUE(call_stmt != nullptr);

  EXPECT_EQ(call_stmt->identifier, "a");
  EXPECT_EQ(call_stmt->arguments.size(), 0);
}

TEST(ParserTest, func_decl_stmt) {
  StringSource source("void func(int a) { print a; }");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto func_stmt = dynamic_cast<FuncStmt*>(program->statements[0].get());
  EXPECT_TRUE(func_stmt != nullptr);
  EXPECT_EQ(func_stmt->identifier, "func");
  EXPECT_EQ(func_stmt->return_type.type, VOID);

  auto& params = func_stmt->params;
  EXPECT_EQ(params.size(), 1);
  auto param = dynamic_cast<FuncParamStmt*>(params[0].get());
  EXPECT_TRUE(param != nullptr);
  EXPECT_EQ(param->type.type, INT);
  EXPECT_EQ(param->identifier, "a");

  auto body = dynamic_cast<BlockStmt*>(func_stmt->body.get());
  EXPECT_TRUE(body != nullptr);
  EXPECT_EQ(body->statements.size(), 1);
  EXPECT_TRUE(dynamic_cast<PrintStmt*>(body->statements[0].get()) != nullptr);
}
