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
  EXPECT_EQ(fieldaccess_expr->field_name.get_type(),
            TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(fieldaccess_expr->field_name.stringify(), "b");

  auto parent_struct =
      dynamic_cast<VarExpr*>(fieldaccess_expr->parent_struct.get());
  EXPECT_TRUE(parent_struct != nullptr);
  EXPECT_EQ(parent_struct->identifier, "a");
}

class ParserTypeCastExprTest
    : public ::testing::TestWithParam<std::pair<std::string, TokenType>> {};

TEST_P(ParserTypeCastExprTest, print_type_cast) {
  StringSource source("print a " + GetParam().first + " int;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto cast_expr = dynamic_cast<CastExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(cast_expr != nullptr);
  EXPECT_EQ(cast_expr->op_symbol.get_type(), GetParam().second);
  EXPECT_EQ(cast_expr->type.get_type(), TokenType::TOKEN_INT);

  auto left = dynamic_cast<VarExpr*>(cast_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->identifier, "a");
}

INSTANTIATE_TEST_SUITE_P(
    ParserTypeCastExprParams, ParserTypeCastExprTest,
    ::testing::Values(std::make_pair("is", TokenType::TOKEN_IS),
                      std::make_pair("as", TokenType::TOKEN_AS)));

class ParserUnaryExprTest
    : public ::testing::TestWithParam<std::pair<std::string, TokenType>> {};

TEST_P(ParserUnaryExprTest, print_unary) {
  StringSource source("print " + GetParam().first + "a;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto unary_expr = dynamic_cast<UnaryExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(unary_expr != nullptr);
  EXPECT_EQ(unary_expr->op_symbol.get_type(), GetParam().second);

  auto left = dynamic_cast<VarExpr*>(unary_expr->right.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->identifier, "a");
}

INSTANTIATE_TEST_SUITE_P(
    ParserUnaryExprParams, ParserUnaryExprTest,
    ::testing::Values(std::make_pair("-", TokenType::TOKEN_MINUS),
                      std::make_pair("!", TokenType::TOKEN_EXCLAMATION)));

class ParserBinaryExprTest
    : public ::testing::TestWithParam<std::pair<std::string, TokenType>> {};

TEST_P(ParserBinaryExprTest, print_binary) {
  StringSource source("print 1 " + GetParam().first + " 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<BinaryExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
  EXPECT_EQ(binary_expr->op_symbol.get_type(), GetParam().second);

  auto left = dynamic_cast<LiteralExpr*>(binary_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->literal.get_type(), TokenType::TOKEN_INT_VAL);
  EXPECT_EQ(left->literal.stringify(), "1");

  auto right = dynamic_cast<LiteralExpr*>(binary_expr->right.get());
  EXPECT_TRUE(right != nullptr);
  EXPECT_EQ(right->literal.get_type(), TokenType::TOKEN_INT_VAL);
  EXPECT_EQ(right->literal.stringify(), "2");
}

INSTANTIATE_TEST_SUITE_P(
    ParserBinaryExprParams, ParserBinaryExprTest,
    ::testing::Values(std::make_pair("!=", TokenType::TOKEN_NOT_EQUAL),
                      std::make_pair("==", TokenType::TOKEN_EQUAL_EQUAL),
                      std::make_pair(">", TokenType::TOKEN_GREATER),
                      std::make_pair(">=", TokenType::TOKEN_GREATER_EQUAL),
                      std::make_pair("<", TokenType::TOKEN_LESS),
                      std::make_pair("<=", TokenType::TOKEN_LESS_EQUAL),
                      std::make_pair("-", TokenType::TOKEN_MINUS),
                      std::make_pair("+", TokenType::TOKEN_PLUS),
                      std::make_pair("/", TokenType::TOKEN_SLASH),
                      std::make_pair("*", TokenType::TOKEN_STAR)));

class ParserLogicalExprTest
    : public ::testing::TestWithParam<std::pair<std::string, TokenType>> {};

TEST_P(ParserLogicalExprTest, print_logical) {
  StringSource source("print 1 " + GetParam().first + " 2;");
  Lexer lexer(source);
  Parser parser(lexer);
  auto program = parser.parse();
  EXPECT_EQ(program->statements.size(), 1);

  auto print_stmt = dynamic_cast<PrintStmt*>(program->statements[0].get());
  EXPECT_TRUE(print_stmt != nullptr);

  auto binary_expr = dynamic_cast<LogicalExpr*>(print_stmt->expr.get());
  EXPECT_TRUE(binary_expr != nullptr);
  EXPECT_EQ(binary_expr->op_symbol.get_type(), GetParam().second);

  auto left = dynamic_cast<LiteralExpr*>(binary_expr->left.get());
  EXPECT_TRUE(left != nullptr);
  EXPECT_EQ(left->literal.get_type(), TokenType::TOKEN_INT_VAL);
  EXPECT_EQ(left->literal.stringify(), "1");

  auto right = dynamic_cast<LiteralExpr*>(binary_expr->right.get());
  EXPECT_TRUE(right != nullptr);
  EXPECT_EQ(right->literal.get_type(), TokenType::TOKEN_INT_VAL);
  EXPECT_EQ(right->literal.stringify(), "2");
}

INSTANTIATE_TEST_SUITE_P(
    ParserLogicalExprParams, ParserLogicalExprTest,
    ::testing::Values(std::make_pair("and", TokenType::TOKEN_AND),
                      std::make_pair("or", TokenType::TOKEN_OR)));
