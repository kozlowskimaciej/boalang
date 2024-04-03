#include <gtest/gtest.h>

#include "lexer/lexer.hpp"
#include "token/token.hpp"

class LexerTokenizeTest : public ::testing::Test {
 protected:
  StringSource source_ = {};
  Lexer lexer_;

  LexerTokenizeTest() : lexer_({source_}){};
};

bool str_contains(const std::string& str, const std::string& str2) {
  return str.find(str2) != std::string::npos;
}

TEST_F(LexerTokenizeTest, integer_valid) {
  source_.attach("2147483647");

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_INT_VAL);
  EXPECT_TRUE(std::holds_alternative<int>(token.value.value()));
  EXPECT_EQ(std::get<int>(token.value.value()), 2147483647);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, integer_overflow) {
  source_.attach("2147483648");

  EXPECT_THROW(
      {
        try {
          lexer_.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(
              e.what(), "Int literal exceeds maximum value (2147483647)"));
          throw;
        }
      },
      LexerError);
}

TEST_F(LexerTokenizeTest, integer_leading_zeros) {
  source_.attach("001");

  EXPECT_THROW(
      {
        try {
          lexer_.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Leading zeros are not allowed"));
          throw;
        }
      },
      LexerError);
}

TEST_F(LexerTokenizeTest, float_valid) {
  source_.attach("1.01");

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_FLOAT_VAL);
  EXPECT_TRUE(std::holds_alternative<float>(token.value.value()));
  ASSERT_FLOAT_EQ(std::get<float>(token.value.value()), 1.01F);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, float_no_digit_after_dot) {
  source_.attach("1..");

  EXPECT_THROW(
      {
        try {
          lexer_.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Expected digit after '.'"));
          throw;
        }
      },
      LexerError);
}

TEST_F(LexerTokenizeTest, comment_valid) {
  source_.attach("void//void\nvoid");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VOID);

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_COMMENT);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value.value()));
  EXPECT_EQ(std::get<std::string>(token.value.value()), "void");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, long_comment_valid) {
  source_.attach("void/*void\nvoid*/void");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VOID);

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_COMMENT);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value.value()));
  EXPECT_EQ(std::get<std::string>(token.value.value()), "void\nvoid");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, long_comment_unterminated) {
  source_.attach("void/*void\nvoid");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_THROW(
      {
        try {
          lexer_.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), "Unterminated long comment"));
          throw;
        }
      },
      LexerError);
}

TEST_F(LexerTokenizeTest, identifier_valid) {
  std::string id = "_variable1";
  source_.attach(id);

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(token.value.value()), id);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, identifier_too_long) {
  std::string id = std::string(MAX_IDENTIFIER_LENGTH + 1, 'A');
  source_.attach(id);

  EXPECT_THROW(
      {
        try {
          lexer_.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), id));
          EXPECT_TRUE(
              str_contains(e.what(), "Identifier exceeds maximum length"));
          throw;
        }
      },
      LexerError);
}

TEST_F(LexerTokenizeTest, string) {
  source_.attach("\"Hello World!\"");

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_STR_VAL);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value.value()));
  EXPECT_EQ(std::get<std::string>(token.value.value()), "Hello World!");
}

TEST_F(LexerTokenizeTest, string_unterminated) {
  std::string str = "\"Hello World!";
  source_.attach(str);

  EXPECT_THROW(
      {
        try {
          lexer_.next_token();
        } catch (const LexerError& e) {
          EXPECT_TRUE(str_contains(e.what(), str));
          EXPECT_TRUE(str_contains(e.what(), "Unterminated string"));
          throw;
        }
      },
      LexerError);
}

TEST_F(LexerTokenizeTest, tokenize_sample_code) {
  source_.attach(
      "struct S {\n"
      "    mut int a;\n"
      "    float b;\n"
      "}\n"
      "\n"
      "mut S st_obj = {121.5, 10};\n"
      "st_obj.a = st_obj.b as int;\n"
      "\"hello\";");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_STRUCT);
  Token S_id = lexer_.next_token();
  EXPECT_EQ(S_id.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(S_id.value.value()), "S");
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_LBRACE);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_MUT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_INT);
  Token a_id = lexer_.next_token();
  EXPECT_EQ(a_id.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(a_id.value.value()), "a");
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_FLOAT);
  Token b_id = lexer_.next_token();
  EXPECT_EQ(b_id.type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(b_id.value.value()), "b");
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_RBRACE);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_MUT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_LBRACE);
  Token float_val = lexer_.next_token();
  EXPECT_EQ(float_val.type, TokenType::TOKEN_FLOAT_VAL);
  ASSERT_FLOAT_EQ(std::get<float>(float_val.value.value()), 121.5F);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_COMMA);
  Token int_val = lexer_.next_token();
  EXPECT_EQ(int_val.type, TokenType::TOKEN_INT_VAL);
  ASSERT_EQ(std::get<int>(int_val.value.value()), 10);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_RBRACE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_DOT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_DOT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IDENTIFIER);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_AS);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_INT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);

  Token str_val = lexer_.next_token();
  EXPECT_EQ(str_val.type, TokenType::TOKEN_STR_VAL);
  EXPECT_EQ(std::get<std::string>(str_val.value.value()), "hello");
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

class LexerTokenizeParamTest
    : public LexerTokenizeTest,
      public ::testing::WithParamInterface<std::pair<std::string, TokenType>> {
};

TEST_P(LexerTokenizeParamTest, tokenize_chars) {
  source_.attach(GetParam().first);
  EXPECT_EQ(lexer_.next_token().type, GetParam().second);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
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
