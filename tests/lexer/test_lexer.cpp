#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "token/token.hpp"

bool str_contains(const std::string& str, const std::string& str2) {
  return str.find(str2) != std::string::npos;
}

TEST(LexerTokenizeTest, integer_valid) {
  StringSource source("2147483647");
  Lexer lexer(source);

  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_INT_VAL);
  EXPECT_TRUE(std::holds_alternative<int>(token.value));
  EXPECT_EQ(std::get<int>(token.value), 2147483647);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

TEST(LexerTokenizeTest, integer_overflow) {
  StringSource source("2147483648");
  Lexer lexer(source);

  EXPECT_THROW(
      {
        try {
          lexer.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(
              e.what(), "Int literal exceeds maximum value (2147483647)"));
          throw;
        }
      },
      LexerError);
}

TEST(LexerTokenizeTest, integer_leading_zeros) {
  StringSource source("001");
  Lexer lexer(source);

  EXPECT_THROW(
      {
        try {
          lexer.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Leading zeros are not allowed"));
          throw;
        }
      },
      LexerError);
}

class LexerFloatValidTests
    : public ::testing::TestWithParam<std::string> {};

TEST_P(LexerFloatValidTests, tokenize_float_valid) {
  StringSource source(GetParam());
  Lexer lexer(source);

  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_FLOAT_VAL);
  EXPECT_TRUE(std::holds_alternative<float>(token.value));
  ASSERT_FLOAT_EQ(std::get<float>(token.value), std::stof(GetParam()));

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

INSTANTIATE_TEST_SUITE_P(
    LexerFloatTests, LexerFloatValidTests,
    ::testing::Values("0.2147483647", "214748364.0", "21474.83647"));

class LexerFloatOverflowTests
    : public ::testing::TestWithParam<std::string> {};

TEST_P(LexerFloatOverflowTests, tokenize_float_overflow) {
  StringSource source(GetParam());
  Lexer lexer(source);

  EXPECT_THROW(
      {
        try {
          lexer.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Float literal exceeds range"));
          throw;
        }
      },
      LexerError);
}

INSTANTIATE_TEST_SUITE_P(
    LexerFloatTests, LexerFloatOverflowTests,
    ::testing::Values("0.00000000001", "0.2147483648", "0.02147483647", "214748364.8", "21474.83648"));

TEST(LexerTokenizeTest, comment_valid) {
  StringSource source("void//void\nvoid");
  Lexer lexer(source);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_VOID);

  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_COMMENT);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value));
  EXPECT_EQ(std::get<std::string>(token.value), "void");

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

TEST(LexerTokenizeTest, long_comment_valid) {
  StringSource source("void/*void\nvoid*/void");
  Lexer lexer(source);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_VOID);

  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_COMMENT);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value));
  EXPECT_EQ(std::get<std::string>(token.value), "void\nvoid");

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

TEST(LexerTokenizeTest, long_comment_unterminated) {
  StringSource source("void/*void\nvoid");
  Lexer lexer(source);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_THROW(
      {
        try {
          lexer.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Unterminated long comment"));
          throw;
        }
      },
      LexerError);
}

TEST(LexerTokenizeTest, identifier_valid) {
  std::string id = "_variable1";
  StringSource source(id);
  Lexer lexer(source);

  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(token.value), id);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

TEST(LexerTokenizeTest, identifier_too_long) {
  std::string id = std::string(MAX_IDENTIFIER_LENGTH + 1, 'A');
  StringSource source(id);
  Lexer lexer(source);

  EXPECT_THROW(
      {
        try {
          lexer.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), id));
          EXPECT_TRUE(
              str_contains(e.what(), "Identifier exceeds maximum length"));
          throw;
        }
      },
      LexerError);
}

TEST(LexerTokenizeTest, string) {
  StringSource source("\"Hello World!\"");
  Lexer lexer(source);

  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_STR_VAL);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value));
  EXPECT_EQ(std::get<std::string>(token.value), "Hello World!");
}

TEST(LexerTokenizeTest, string_unterminated) {
  std::string str = "\"Hello World!";
  StringSource source(str);
  Lexer lexer(source);

  EXPECT_THROW(
      {
        try {
          lexer.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), str));
          EXPECT_TRUE(str_contains(e.what(), "Unterminated string"));
          throw;
        }
      },
      LexerError);
}

TEST(LexerTokenizeTest, tokenize_sample_code) {
  StringSource source(
      "struct S {\n"
      "    mut int a;\n"
      "    float b;\n"
      "}\n"
      "\n"
      "mut S st_obj = {121.5, 10};\n"
      "st_obj.a = st_obj.b as int;\n"
      "\"hello\";");
  Lexer lexer(source);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_STRUCT);
  Token S_id = lexer.next_token();
  EXPECT_EQ(S_id.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(S_id.value), "S");
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_LBRACE);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_MUT);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_INT);
  Token a_id = lexer.next_token();
  EXPECT_EQ(a_id.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(a_id.value), "a");
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_FLOAT);
  Token b_id = lexer.next_token();
  EXPECT_EQ(b_id.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(b_id.value), "b");
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_RBRACE);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_MUT);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_EQUAL);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_LBRACE);
  Token float_val = lexer.next_token();
  EXPECT_EQ(float_val.type, TokenType::TOKEN_FLOAT_VAL);
  ASSERT_FLOAT_EQ(std::get<float>(float_val.value), 121.5F);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_COMMA);
  Token int_val = lexer.next_token();
  EXPECT_EQ(int_val.type, TokenType::TOKEN_INT_VAL);
  ASSERT_EQ(std::get<int>(int_val.value), 10);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_RBRACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_DOT);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_EQUAL);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_DOT);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_AS);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_INT);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_SEMICOLON);

  Token str_val = lexer.next_token();
  EXPECT_EQ(str_val.type, TokenType::TOKEN_STR_VAL);
  EXPECT_EQ(std::get<std::string>(str_val.value), "hello");
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

class LexerTokenizeParamTest
    : public ::testing::TestWithParam<std::pair<std::string, TokenType>> {};

TEST_P(LexerTokenizeParamTest, tokenize_chars) {
  StringSource source(GetParam().first);
  Lexer lexer(source);
  EXPECT_EQ(lexer.next_token().type, GetParam().second);
  EXPECT_EQ(lexer.next_token().type, TokenType::TOKEN_ETX);
}

INSTANTIATE_TEST_SUITE_P(
    LexerSingleTokensTests, LexerTokenizeParamTest,
    ::testing::Values(std::make_pair("", TokenType::TOKEN_ETX),
                      std::make_pair("(", TokenType::TOKEN_LPAREN),
                      std::make_pair(")", TokenType::TOKEN_RPAREN),
                      std::make_pair("{", TokenType::TOKEN_LBRACE),
                      std::make_pair("}", TokenType::TOKEN_RBRACE),
                      std::make_pair(",", TokenType::TOKEN_COMMA),
                      std::make_pair(".", TokenType::TOKEN_DOT),
                      std::make_pair("-", TokenType::TOKEN_MINUS),
                      std::make_pair("+", TokenType::TOKEN_PLUS),
                      std::make_pair(";", TokenType::TOKEN_SEMICOLON),
                      std::make_pair("/", TokenType::TOKEN_SLASH),
                      std::make_pair("*", TokenType::TOKEN_STAR),
                      std::make_pair("!", TokenType::TOKEN_EXCLAMATION),
                      std::make_pair("=", TokenType::TOKEN_EQUAL),
                      std::make_pair("<", TokenType::TOKEN_LESS),
                      std::make_pair(">", TokenType::TOKEN_GREATER),
                      std::make_pair("identifier", TokenType::TOKEN_IDENTIFIER),
                      std::make_pair("muta", TokenType::TOKEN_IDENTIFIER),
                      std::make_pair("\"123\"", TokenType::TOKEN_STR_VAL),
                      std::make_pair("123", TokenType::TOKEN_INT_VAL),
                      std::make_pair("123.0", TokenType::TOKEN_FLOAT_VAL),
                      std::make_pair("mut", TokenType::TOKEN_MUT),
                      std::make_pair("if", TokenType::TOKEN_IF),
                      std::make_pair("else", TokenType::TOKEN_ELSE),
                      std::make_pair("and", TokenType::TOKEN_AND),
                      std::make_pair("or", TokenType::TOKEN_OR),
                      std::make_pair("true", TokenType::TOKEN_TRUE),
                      std::make_pair("false", TokenType::TOKEN_FALSE),
                      std::make_pair("while", TokenType::TOKEN_WHILE),
                      std::make_pair("return", TokenType::TOKEN_RETURN),
                      std::make_pair("is", TokenType::TOKEN_IS),
                      std::make_pair("as", TokenType::TOKEN_AS),
                      std::make_pair("print", TokenType::TOKEN_PRINT),
                      std::make_pair("inspect", TokenType::TOKEN_INSPECT),
                      std::make_pair("struct", TokenType::TOKEN_STRUCT),
                      std::make_pair("variant", TokenType::TOKEN_VARIANT),
                      std::make_pair("int", TokenType::TOKEN_INT),
                      std::make_pair("float", TokenType::TOKEN_FLOAT),
                      std::make_pair("str", TokenType::TOKEN_STR),
                      std::make_pair("bool", TokenType::TOKEN_BOOL),
                      std::make_pair("void", TokenType::TOKEN_VOID),
                      std::make_pair("==", TokenType::TOKEN_EQUAL_EQUAL),
                      std::make_pair("!=", TokenType::TOKEN_NOT_EQUAL),
                      std::make_pair("<=", TokenType::TOKEN_LESS_EQUAL),
                      std::make_pair(">=", TokenType::TOKEN_GREATER_EQUAL),
                      std::make_pair("=>", TokenType::TOKEN_ARROW)));
