#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include <optional>

#include "../source/source.hpp"
#include "../token/token.hpp"

constexpr unsigned int MAX_IDENTIFIER_LENGTH = 64;
using opt_token_t = std::optional<Token>;

class ILexer {
 public:
  virtual ~ILexer() = default;
  virtual Token next_token() = 0;
};

class Lexer : public ILexer {
 private:
  std::string current_context_;
  Source& source_;
  [[nodiscard]] Token build_token_with_value(const TokenType& type) const;
  [[nodiscard]] Token build_token_with_value(const TokenType& type,
                                             const token_value_t& value) const;
  [[nodiscard]] Token build_token(const TokenType& type) const;
  char advance();
  bool match(char c);
  opt_token_t handle_single_char_token();
  opt_token_t handle_double_char_token();
  opt_token_t handle_slash_token();
  opt_token_t try_tokenize_string();
  opt_token_t try_tokenize_number();
  opt_token_t try_tokenize_identifier();
  opt_token_t try_tokenize_comment();
  void skip_whitespace();
  Token build_fraction(int value);

 public:
  explicit Lexer(Source& source) : source_(source){};
  Token next_token() override;
};

class LexerCommentFilter : public ILexer {
  ILexer& lexer_;

 public:
  explicit LexerCommentFilter(ILexer& lexer) : lexer_(lexer){};
  Token next_token() override;
};

class LexerError : public std::exception {
  Token token_;
  std::string message_;

 public:
  LexerError(const Token& token, const std::string& message) : token_(token) {
    message_ = "Line " + std::to_string(token.position.line) + " column " +
               std::to_string(token.position.column) + " at '" +
               token.stringify() + "': " + message;
  };

  [[nodiscard]] const char* what() const noexcept override {
    return message_.c_str();
  }

  [[nodiscard]] const Token& get_token() const { return token_; }
};

#endif  // BOALANG_LEXER_HPP
