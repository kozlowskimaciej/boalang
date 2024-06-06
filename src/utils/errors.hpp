/*! @file errors.hpp
    @brief Errors.
*/

#ifndef BOALANG_ERRORS_HPP
#define BOALANG_ERRORS_HPP

#include <stdexcept>
#include <string>

#include "token/token.hpp"
#include "utils/position.hpp"

/**
 * @brief Represents a lexer related error.
 */
class LexerError : public std::runtime_error {
  Token token_;

 public:
  LexerError(const Token& token, const std::string& message)
      : runtime_error("Line " + std::to_string(token.get_position().line) +
                      " column " + std::to_string(token.get_position().column) +
                      " at '" + token.stringify() + "': " + message),
        token_(token){};

  [[nodiscard]] const Token& get_token() const { return token_; }
};

/**
 * @brief Represents a parser related error.
 */
class SyntaxError : public std::runtime_error {
  Token token_;

 public:
  SyntaxError(const Token& token, const std::string& message)
      : runtime_error("Line " + std::to_string(token.get_position().line) +
                      " column " + std::to_string(token.get_position().column) +
                      " at '" +
                      (token.stringify().empty() ? token.stringify_type()
                                                 : token.stringify()) +
                      "': " + message),
        token_(token){};

  [[nodiscard]] const Token& get_token() const { return token_; }
};

/**
 * @brief Represents an interpreter related error.
 */
class RuntimeError : public std::runtime_error {
 public:
  RuntimeError(const std::string& message) : std::runtime_error(message){};
  RuntimeError(const Position& position, const std::string& message)
      : std::runtime_error("Line " + std::to_string(position.line) +
                           " column " + std::to_string(position.column) + ": " +
                           message){};
};

#endif  // BOALANG_ERRORS_HPP
