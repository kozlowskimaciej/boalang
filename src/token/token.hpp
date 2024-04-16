#ifndef BOALANG_TOKEN_HPP
#define BOALANG_TOKEN_HPP

#include <ostream>
#include <string>
#include <variant>

#include "utils/position.hpp"

using token_value_t =
    std::variant<std::monostate, std::string, int, float, bool>;

enum TokenType {
  // END OF TEXT
  TOKEN_ETX = 0,

  // SINGLE CHAR
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,
  TOKEN_EXCLAMATION,
  TOKEN_EQUAL,
  TOKEN_LESS,
  TOKEN_GREATER,

  // DOUBLE CHARS
  TOKEN_EQUAL_EQUAL,
  TOKEN_NOT_EQUAL,
  TOKEN_LESS_EQUAL,
  TOKEN_GREATER_EQUAL,
  TOKEN_ARROW,  // '=>'

  // LITERALS
  TOKEN_IDENTIFIER,
  TOKEN_STR_VAL,
  TOKEN_INT_VAL,
  TOKEN_FLOAT_VAL,

  // KEYWORDS
  TOKEN_MUT,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_WHILE,
  TOKEN_RETURN,
  TOKEN_IS,
  TOKEN_AS,
  TOKEN_PRINT,
  TOKEN_INSPECT,
  TOKEN_STRUCT,
  TOKEN_VARIANT,
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_STR,
  TOKEN_BOOL,
  TOKEN_VOID,

  TOKEN_COMMENT,
  TOKEN_UNKNOWN,
};

class Token {
  TokenType type;
  token_value_t value;
  Position position;

 public:
  Token(TokenType type, Position position) : type(type), position(position){};
  Token(TokenType type, token_value_t value, Position position)
      : type(type), value(std::move(value)), position(position){};

  [[nodiscard]] const TokenType& get_type() const { return type; };
  [[nodiscard]] const token_value_t& get_value() const { return value; };
  [[nodiscard]] const Position& get_position() const { return position; };

  [[nodiscard]] std::string stringify() const;
  [[nodiscard]] bool has_value() const;

  friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

#endif  // BOALANG_TOKEN_HPP
