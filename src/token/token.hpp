#ifndef TKOM_BOALANG_TOKEN_HPP
#define TKOM_BOALANG_TOKEN_HPP

#include <map>
#include <optional>
#include <string>
#include <variant>

#include "../utils/position.hpp"

using token_value_t = std::optional<std::variant<std::wstring, int, float, bool>>;

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
  TOKEN_STRVAL,
  TOKEN_INTVAL,
  TOKEN_FLOATVAL,

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
    {"if", TOKEN_IF},         {"else", TOKEN_ELSE},
    {"and", TOKEN_AND},       {"or", TOKEN_OR},
    {"true", TOKEN_TRUE},     {"false", TOKEN_FALSE},
    {"while", TOKEN_WHILE},   {"return", TOKEN_RETURN},
    {"is", TOKEN_IS},         {"as", TOKEN_AS},
    {"print", TOKEN_PRINT},   {"inspect", TOKEN_INSPECT},
    {"struct", TOKEN_STRUCT}, {"variant", TOKEN_VARIANT},
    {"int", TOKEN_INT},       {"float", TOKEN_FLOAT},
    {"str", TOKEN_STR},       {"bool", TOKEN_BOOL},
    {"void", TOKEN_VOID},     {"mut", TOKEN_MUT},
};

class Token {
 public:
  const TokenType type;
  const token_value_t value;
  const Position position;

  Token(TokenType type, Position position)
      : type(type), value(std::nullopt), position(position){};
  Token(TokenType type, token_value_t value, Position position)
      : type(type), value(std::move(value)), position(position){};

  [[nodiscard]] std::wstring stringify() const;
};

#endif  // TKOM_BOALANG_TOKEN_HPP
