#ifndef BOALANG_TOKEN_HPP
#define BOALANG_TOKEN_HPP

#include <map>
#include <optional>
#include <string>
#include <variant>

#include "../utils/position.hpp"

using token_value_t =
    std::optional<std::variant<std::wstring, int, float, bool>>;

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

  TOKEN_UNKNOWN
};

static const std::map<TokenType, std::wstring> token_repr{
    {TOKEN_ETX, L"ETX"},
    {TOKEN_LPAREN, L"LPAREN"},
    {TOKEN_RPAREN, L"RPAREN"},
    {TOKEN_LBRACE, L"LBRACE"},
    {TOKEN_RBRACE, L"RBRACE"},
    {TOKEN_COMMA, L"COMMA"},
    {TOKEN_DOT, L"DOT"},
    {TOKEN_MINUS, L"MINUS"},
    {TOKEN_PLUS, L"PLUS"},
    {TOKEN_SEMICOLON, L"SEMICOLON"},
    {TOKEN_SLASH, L"SLASH"},
    {TOKEN_STAR, L"STAR"},
    {TOKEN_EXCLAMATION, L"EXCLAMATION"},
    {TOKEN_EQUAL, L"EQUAL"},
    {TOKEN_LESS, L"LESS"},
    {TOKEN_GREATER, L"GREATER"},
    {TOKEN_EQUAL_EQUAL, L"EQUAL_EQUAL"},
    {TOKEN_NOT_EQUAL, L"NOT_EQUAL"},
    {TOKEN_LESS_EQUAL, L"LESS_EQUAL"},
    {TOKEN_GREATER_EQUAL, L"GREATER_EQUAL"},
    {TOKEN_ARROW, L"ARROW"},
    {TOKEN_IDENTIFIER, L"IDENTIFIER"},
    {TOKEN_STR_VAL, L"STR_VAL"},
    {TOKEN_INT_VAL, L"INT_VAL"},
    {TOKEN_FLOAT_VAL, L"FLOAT_VAL"},
    {TOKEN_MUT, L"MUT"},
    {TOKEN_IF, L"IF"},
    {TOKEN_ELSE, L"ELSE"},
    {TOKEN_AND, L"AND"},
    {TOKEN_OR, L"OR"},
    {TOKEN_TRUE, L"TRUE"},
    {TOKEN_FALSE, L"FALSE"},
    {TOKEN_WHILE, L"WHILE"},
    {TOKEN_RETURN, L"RETURN"},
    {TOKEN_IS, L"IS"},
    {TOKEN_AS, L"AS"},
    {TOKEN_PRINT, L"PRINT"},
    {TOKEN_INSPECT, L"INSPECT"},
    {TOKEN_STRUCT, L"STRUCT"},
    {TOKEN_VARIANT, L"VARIANT"},
    {TOKEN_INT, L"INT"},
    {TOKEN_FLOAT, L"FLOAT"},
    {TOKEN_STR, L"STR"},
    {TOKEN_BOOL, L"BOOL"},
    {TOKEN_VOID, L"VOID"},
    {TOKEN_UNKNOWN, L"UNKNOWN"},
};

static const std::map<std::wstring, TokenType> keywords{
    {L"if", TOKEN_IF},         {L"else", TOKEN_ELSE},
    {L"and", TOKEN_AND},       {L"or", TOKEN_OR},
    {L"true", TOKEN_TRUE},     {L"false", TOKEN_FALSE},
    {L"while", TOKEN_WHILE},   {L"return", TOKEN_RETURN},
    {L"is", TOKEN_IS},         {L"as", TOKEN_AS},
    {L"print", TOKEN_PRINT},   {L"inspect", TOKEN_INSPECT},
    {L"struct", TOKEN_STRUCT}, {L"variant", TOKEN_VARIANT},
    {L"int", TOKEN_INT},       {L"float", TOKEN_FLOAT},
    {L"str", TOKEN_STR},       {L"bool", TOKEN_BOOL},
    {L"void", TOKEN_VOID},     {L"mut", TOKEN_MUT},
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

  friend std::wostream& operator<<(std::wostream& os, const Token& token);
};

#endif  // BOALANG_TOKEN_HPP
