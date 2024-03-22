#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include <codecvt>
#include <locale>

#include "../source/source.hpp"
#include "../token/token.hpp"

class Lexer {
 private:
  Source& source_;
  [[nodiscard]] Token build_token(
      const TokenType& type, const token_value_t& value = std::nullopt) const;
  Token tokenize_string();

 public:
  explicit Lexer(Source& source) : source_(source){};
  Token next_token();
};

class LexerError : public std::exception {
  const std::wstring message_;
  std::string narrow_message_;

 public:
  LexerError(const Token& token, const std::wstring& message)
      : message_(L"Line " + std::to_wstring(token.position.line) + L" column " +
                 std::to_wstring(token.position.column) + L" at '" +
                 token.stringify() + L"': " + message) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    narrow_message_ = converter.to_bytes(message_);
  };

  [[nodiscard]] const char* what() const noexcept override {
    return narrow_message_.c_str();
  }

  [[nodiscard]] const std::wstring& what_utf8() const noexcept {
    return message_;
  }
};

#endif  // BOALANG_LEXER_HPP
