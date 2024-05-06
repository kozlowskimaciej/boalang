#include <gtest/gtest.h>

#include "../utils.hpp"
#include "parser/parser.hpp"

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
  EXPECT_EQ(expr->literal.get_type(), TOKEN_STR_VAL);
  EXPECT_EQ(expr->literal.stringify(), "Hello World");
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
  EXPECT_EQ(expr->literal.get_type(), TOKEN_INT_VAL);
  EXPECT_EQ(expr->literal.stringify(), "1");
}

TEST(ParserTest, print_primary_float_val) {
  StringSource source("print 1.1;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto expr = dynamic_cast<LiteralExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(expr->literal.get_type(), TOKEN_FLOAT_VAL);
  EXPECT_TRUE(str_contains(expr->literal.stringify(), "1.1"));
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
  EXPECT_EQ(expr->literal.get_type(), TOKEN_TRUE);
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
  EXPECT_EQ(expr->literal.get_type(), TOKEN_INT_VAL);
  EXPECT_EQ(expr->literal.stringify(), "1");
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
  EXPECT_EQ(expr->literal.get_type(), TOKEN_INT_VAL);
  EXPECT_EQ(expr->literal.stringify(), "1");

  expr = dynamic_cast<LiteralExpr*>(initalizerlist_expr->list[1].get());
  EXPECT_TRUE(expr != nullptr);
  EXPECT_EQ(expr->literal.get_type(), TOKEN_INT_VAL);
  EXPECT_EQ(expr->literal.stringify(), "2");
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
  EXPECT_EQ(arg->literal.get_type(), TOKEN_INT_VAL);
  EXPECT_EQ(arg->literal.stringify(), "1");

  arg = dynamic_cast<LiteralExpr*>(call_expr->arguments[1].get());
  EXPECT_TRUE(arg != nullptr);
  EXPECT_EQ(arg->literal.get_type(), TOKEN_INT_VAL);
  EXPECT_EQ(arg->literal.stringify(), "2");

  auto callee = dynamic_cast<VarExpr*>(call_expr->callee.get());
  EXPECT_TRUE(callee != nullptr);
  EXPECT_EQ(callee->identifier, "foo");
}

TEST(ParserTest, print_call_function_args_over_limit) {
  std::string code = "print foo(";
  for (size_t i = 0; i <= 256; ++i) {
    code += "1, ";
  }
  code += "1);";
  StringSource source(code);
  Lexer lexer(source);
  Parser parser(lexer);
  EXPECT_THROW({ parser.parse(); }, SyntaxError);
}