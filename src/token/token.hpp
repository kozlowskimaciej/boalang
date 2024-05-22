/*! @file token.hpp
    @brief Implementation of Token class and related types.
*/

#ifndef BOALANG_TOKEN_HPP
#define BOALANG_TOKEN_HPP

#include <ostream>
#include <string>
#include <variant>

#include "utils/position.hpp"

using token_value_t =
    std::variant<std::monostate, std::string, int, float,
                 bool>; /**< Variant of all available value types. */

/**
 * @brief Represents all available token types.
 */
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
  TOKEN_DEFAULT,

  TOKEN_COMMENT,
  TOKEN_UNKNOWN,
};

/**
 * @brief Represents a token.
 *
 * Holds type, value and position in source.
 */
class Token {
  TokenType type;      /**< Token's type. */
  token_value_t value; /**< Stored value. */
  Position position;   /**< Position in source. */

 public:
  /**
   * @brief Construct a new Token object with no value.
   *
   * @param type token's type
   * @param position token's position in source
   */
  Token(TokenType type, Position position) : type(type), position(position){};
  /**
   * @brief Construct a new Token object with value.
   *
   * @param type token's type
   * @param value token's value
   * @param position token's position in source
   */
  Token(TokenType type, token_value_t value, Position position)
      : type(type), value(std::move(value)), position(position){};

  [[nodiscard]] const TokenType& get_type() const { return type; };
  [[nodiscard]] VarType get_var_type() const;
  [[nodiscard]] const token_value_t& get_value() const { return value; };
  [[nodiscard]] const Position& get_position() const { return position; };

  /**
   * @brief Converts token's value to string.
   *
   * @return token's value as a std::string
   */
  [[nodiscard]] std::string stringify() const;
  [[nodiscard]] std::string stringify_type() const;
  [[nodiscard]] bool has_value() const;

  friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

#endif  // BOALANG_TOKEN_HPP
