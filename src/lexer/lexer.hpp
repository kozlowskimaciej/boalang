#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include <codecvt>
#include <locale>

#include "../source/source.hpp"
#include "../token/token.hpp"

class Lexer {
 private:
  const unsigned int MAX_IDENTIFIER_LENGTH = 64;
  std::wstring current_context_;
  Source& source_;
  [[nodiscard]] Token build_token_value(
      const TokenType& type, const token_value_t& value = std::nullopt) const;
  [[nodiscard]] Token build_token(const TokenType& type) const;
  wchar_t advance();
  Token tokenize_string();
  Token tokenize_number();
  Token tokenize_identifier();

 public:
  explicit Lexer(Source& source) : source_(source){};
  Token next_token();
};

class LexerError : public std::exception {
  std::wstring message_utf8_;
  std::string message_;

 public:
  LexerError(const Token& token, const std::wstring& message) {
    message_utf8_ = L"Line " + std::to_wstring(token.position.line) + L" column " +
        std::to_wstring(token.position.column) + L" at '" +
        token.stringify() + L"': " + message;
    message_ = std::string(message_utf8_.begin(), message_utf8_.end());
  };

  [[nodiscard]] const char* what() const noexcept override {
    return message_.c_str();
  }

  [[nodiscard]] const std::wstring& what_utf8() const noexcept {
    return message_utf8_;
  }
};

#endif  // BOALANG_LEXER_HPP
