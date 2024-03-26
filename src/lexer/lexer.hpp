#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include "../source/source.hpp"
#include "../token/token.hpp"

class Lexer {
 private:
  const unsigned int MAX_IDENTIFIER_LENGTH = 64;
  std::string current_context_;
  Source& source_;
  [[nodiscard]] Token build_token_with_value(
      const TokenType& type, const token_value_t& value = std::nullopt) const;
  [[nodiscard]] Token build_token(const TokenType& type) const;
  char advance();
  Token tokenize_string();
  Token tokenize_number();
  Token tokenize_identifier();
  Token tokenize_comment();
  Token tokenize_long_comment();

 public:
  explicit Lexer(Source& source) : source_(source){};
  Token next_token();
};

class LexerError : public std::exception {
  std::string message_;

 public:
  LexerError(const Token& token, const std::string& message) {
    message_ = "Line " + std::to_string(token.position.line) + " column " +
               std::to_string(token.position.column) + " at '" +
               token.stringify() + "': " + message;
  };

  [[nodiscard]] const char* what() const noexcept override {
    return message_.c_str();
  }
};

#endif  // BOALANG_LEXER_HPP
