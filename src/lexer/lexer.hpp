/*! @file lexer.hpp
    @brief Lexer-related classes.
*/

#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include <optional>

#include "source/source.hpp"
#include "token/token.hpp"

constexpr unsigned int MAX_IDENTIFIER_LENGTH =
    64; /**< Maximum identifier length supported by lexer. */
using opt_token_t = std::optional<Token>; /**< Optional token type. */

/**
 * @brief Lexer interface.
 */
class ILexer {
 public:
  ILexer() = default;
  virtual ~ILexer() = default;
  ILexer(const ILexer&) = delete;
  ILexer& operator=(const ILexer&) = delete;
  ILexer(ILexer&&) = delete;
  ILexer& operator=(ILexer&&) = delete;

  virtual Token next_token() = 0;
};

/**
 * @brief Lexer used for lazy source tokenization.
 */
class Lexer : public ILexer {
 private:
  std::string current_context_; /**< Stores advanced chars. */
  Source& source_; /**< Reference to the Source object for tokenization. */

  /**
   * @brief Builds a token of the specified type with \ref
   * Lexer.current_context_ as value.
   *
   * @param type The type of the token to build.
   * @return The constructed token.
   */
  [[nodiscard]] Token build_token_with_value(const TokenType& type) const;

  /**
   * @brief Builds a token of the specified type with the given value.
   *
   * @param type The type of the token to build.
   * @param value The value of the token.
   * @return The constructed token.
   */
  [[nodiscard]] Token build_token_with_value(const TokenType& type,
                                             const token_value_t& value) const;
  /**
   * @brief Builds a token of the specified type without a value.
   *
   * @param type The type of the token to build.
   * @return The constructed token.
   */
  [[nodiscard]] Token build_token(const TokenType& type) const;

  /**
   * @brief Returns next character from \ref Lexer.source_.
   * @return Next character.
   */
  char advance();

  /**
   * @brief Checks if peeked character matches \p c.
   *
   * @param c Character to match.
   * @return True if peeked character matches \p c, false otherwise.
   */
  bool match(char c);
  opt_token_t handle_single_char_token();
  opt_token_t handle_double_char_token();
  opt_token_t handle_slash_token();
  opt_token_t try_tokenize_string();
  opt_token_t try_tokenize_number();
  opt_token_t try_tokenize_identifier();
  opt_token_t try_tokenize_comment();
  void skip_whitespace();

  /**
   * @brief Builds a fractional part of a number token.
   *
   * @param value The integer part of the number.
   * @return The constructed token representing the fractional part of the
   * number.
   */
  Token build_fraction(int value);

 public:
  explicit Lexer(Source& source) : source_(source){};

  /**
   * @brief Creates and returns a new token.
   * @return The next token.
   */
  Token next_token() override;
};

/**
 * @brief Decorator class used for filtering out comments.
 */
class LexerCommentFilter : public ILexer {
  ILexer& lexer_; /**< Reference to decorated lexer. */

 public:
  explicit LexerCommentFilter(ILexer& lexer) : lexer_(lexer){};

  /**
   * @brief Calls next_token() on \ref LexerCommentFilter.lexer_ and returns
   * first non-comment token.
   * @return A token that is not a comment.
   */
  Token next_token() override;
};

/**
 * @brief Represents a lexer related error.
 */
class LexerError : public std::runtime_error {
  Token token_;

 public:
  LexerError(const Token& token, const std::string& message)
      : runtime_error("LexerError: Line " +
                      std::to_string(token.get_position().line) + " column " +
                      std::to_string(token.get_position().column) + " at '" +
                      token.stringify() + "': " + message),
        token_(token){};

  [[nodiscard]] const Token& get_token() const { return token_; }
};

#endif  // BOALANG_LEXER_HPP
