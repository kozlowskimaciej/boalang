#ifndef TKOM_BOALANG_TOKEN_HPP
#define TKOM_BOALANG_TOKEN_HPP

#include <map>
#include <string>
#include <variant>

#include "utils/position.hpp"

enum TokenType {
  TOKEN_EOF = 0,

  // SINGLE CHAR
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
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
  TOKEN_VOID
};

static const std::map<std::string, TokenType> keywords{
    {"mut", TOKEN_MUT},         {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},       {"and", TOKEN_AND},
    {"or", TOKEN_OR},           {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},     {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN},   {"is", TOKEN_IS},
    {"as", TOKEN_AS},           {"print", TOKEN_PRINT},
    {"inspect", TOKEN_INSPECT}, {"struct", TOKEN_STRUCT},
    {"variant", TOKEN_VARIANT}, {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT},     {"str", TOKEN_STR},
    {"bool", TOKEN_BOOL},       {"void", TOKEN_VOID}};

class Token {
  typedef std::variant<std::string, int, float, bool> value_t;

 public:
  const TokenType type;
  const value_t value;
  const Position position;

  Token(TokenType type, value_t value, Position position)
      : type(type), value(value), position(position){};
};

#endif  // TKOM_BOALANG_TOKEN_HPP
