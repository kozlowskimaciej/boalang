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
  source_.attach("print \"Hello World!\";\n");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_PRINT);

  Token token = lexer_.next_token();
  EXPECT_EQ(token.type, TokenType::TOKEN_STR_VAL);
  EXPECT_TRUE(std::holds_alternative<std::string>(token.value.value()));
  EXPECT_EQ(std::get<std::string>(token.value.value()), "Hello World!");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, tokenize_chars) {
  source_.attach("( ) { } , . - + ; / ! = * < >");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_LPAREN);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_RPAREN);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_LBRACE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_RBRACE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_COMMA);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_DOT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_MINUS);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_PLUS);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SEMICOLON);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_SLASH);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_EXCLAMATION);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_STAR);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_LESS);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_GREATER);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, tokenize_keywords) {
  source_.attach(
      "mut if else and or "
      "true false while return "
      "is as print inspect struct "
      "variant int float str bool void");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_MUT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IF);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ELSE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_AND);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_OR);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_TRUE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_FALSE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_WHILE);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_RETURN);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_IS);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_AS);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_PRINT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_INSPECT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_STRUCT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VARIANT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_INT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_FLOAT);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_STR);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_BOOL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_VOID);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}

TEST_F(LexerTokenizeTest, tokenize_double_chars) {
  source_.attach("== != <= >= =>");

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_EQUAL_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_NOT_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_LESS_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_GREATER_EQUAL);
  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ARROW);

  EXPECT_EQ(lexer_.next_token().type, TokenType::TOKEN_ETX);
}
