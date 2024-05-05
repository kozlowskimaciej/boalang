#include "lexer.hpp"

#include <cmath>
#include <functional>
#include <initializer_list>
#include <vector>

static const std::initializer_list<std::pair<std::string, TokenType>> keywords{
    {"if", TOKEN_IF},           {"else", TOKEN_ELSE},
    {"and", TOKEN_AND},         {"or", TOKEN_OR},
    {"true", TOKEN_TRUE},       {"false", TOKEN_FALSE},
    {"while", TOKEN_WHILE},     {"return", TOKEN_RETURN},
    {"is", TOKEN_IS},           {"as", TOKEN_AS},
    {"print", TOKEN_PRINT},     {"inspect", TOKEN_INSPECT},
    {"struct", TOKEN_STRUCT},   {"variant", TOKEN_VARIANT},
    {"int", TOKEN_INT},         {"float", TOKEN_FLOAT},
    {"str", TOKEN_STR},         {"bool", TOKEN_BOOL},
    {"void", TOKEN_VOID},       {"mut", TOKEN_MUT},
    {"default", TOKEN_DEFAULT},
}; /**< List of all supported keywords. */

static constexpr int BASE =
    10; /**< The base number of the decimal number system. */

Token Lexer::build_token_with_value(const TokenType& type) const {
  return {type, current_context_, source_.get_position()};
}

Token Lexer::build_token_with_value(const TokenType& type,
                                    const token_value_t& value) const {
  return {type, value, source_.get_position()};
}

Token Lexer::build_token(const TokenType& type) const {
  return {type, source_.get_position()};
}

opt_token_t Lexer::try_tokenize_string() {
  if (source_.current() != '"') {
    return std::nullopt;
  }

  while (source_.peek() != '"' && !source_.eof() && source_.peek() != '\n') {
    advance();
  }
  if (source_.peek() != '"') {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Unterminated string");
  }
  advance();  // consume closing quote
  return build_token_with_value(
      TOKEN_STR_VAL,
      current_context_.substr(1, current_context_.length() - 2)  // trim quotes
  );
}

opt_token_t Lexer::try_tokenize_number() {
  char c = source_.current();
  if (!std::isdigit(c)) {
    return std::nullopt;
  }

  if (c == '0' && source_.peek() == '0') {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Leading zeros are not allowed");
  }

  int value = c - '0';

  while (std::isdigit(source_.peek()) || source_.peek() == '.') {
    if (source_.peek() == '.') {
      return build_fraction(value);
    }
    int digit = advance() - '0';
    if (value > (INT_MAX - digit) / BASE) {
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Int literal exceeds maximum value (" +
                           std::to_string(INT_MAX) + ")");
    }
    value *= BASE;
    value += digit;
  }
  return build_token_with_value(TOKEN_INT_VAL, value);
}

Token Lexer::build_fraction(int value) {
  if (source_.peek() != '.') {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Expected '.' before fraction part.");
  }
  advance();
  int exponent = 0;
  while (std::isdigit(source_.peek())) {
    int digit = advance() - '0';
    if (value > (INT_MAX - digit) / BASE || exponent == BASE) {
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Float literal exceeds range (" +
                           std::to_string(INT_MAX) + ".0, 0." +
                           std::to_string(INT_MAX) + ")");
    }
    value *= BASE;
    value += digit;
    ++exponent;
  }
  if (!exponent) {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Expected digit after '.'");
  }
  return build_token_with_value(
      TOKEN_FLOAT_VAL, static_cast<float>(value * std::pow(BASE, -exponent)));
}

opt_token_t Lexer::try_tokenize_identifier() {
  if (!std::isalpha(source_.current()) && source_.current() != '_') {
    return std::nullopt;
  }

  while (std::isalnum(source_.peek()) || source_.peek() == '_') {
    advance();
  }
  for (const auto& kw : keywords) {
    if (kw.first == current_context_) {
      return build_token(kw.second);
    }
  }
  if (current_context_.length() > MAX_IDENTIFIER_LENGTH) {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Identifier exceeds maximum length (" +
                         std::to_string(MAX_IDENTIFIER_LENGTH) + ")");
  }
  return build_token_with_value(TOKEN_IDENTIFIER);
}

opt_token_t Lexer::try_tokenize_comment() {
  if (source_.current() != '/') {
    return std::nullopt;
  }

  if (source_.peek() == '/') {  // Single-line comment
    advance();
    while (source_.current() != '\n' && !source_.eof()) {
      advance();
    }
    return build_token_with_value(
        TOKEN_COMMENT,
        current_context_.substr(2, current_context_.length() - 3));
  }
  if (source_.peek() == '*') {  // Multi-line comment
    advance();
    while (!source_.eof()) {
      if (advance() == '*' && match('/')) {
        return build_token_with_value(
            TOKEN_COMMENT,
            current_context_.substr(2, current_context_.length() - 4));
      }
    }
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Unterminated long comment");
  }

  return std::nullopt;
}

opt_token_t Lexer::handle_single_char_token() {
  switch (source_.current()) {
    case '\0':
      return build_token(TOKEN_ETX);
    case '(':
      return build_token(TOKEN_LPAREN);
    case ')':
      return build_token(TOKEN_RPAREN);
    case '{':
      return build_token(TOKEN_LBRACE);
    case '}':
      return build_token(TOKEN_RBRACE);
    case ',':
      return build_token(TOKEN_COMMA);
    case '.':
      return build_token(TOKEN_DOT);
    case '-':
      return build_token(TOKEN_MINUS);
    case '+':
      return build_token(TOKEN_PLUS);
    case ';':
      return build_token(TOKEN_SEMICOLON);
    case '*':
      return build_token(TOKEN_STAR);
    default:
      return std::nullopt;
  }
}

opt_token_t Lexer::handle_double_char_token() {
  switch (source_.current()) {
    case '!':  // '!='
      if (match('=')) {
        return build_token(TOKEN_NOT_EQUAL);
      }
      return build_token(TOKEN_EXCLAMATION);
    case '=':  // '=='
      if (match('=')) {
        return build_token(TOKEN_EQUAL_EQUAL);
      }
      if (match('>')) {
        return build_token(TOKEN_ARROW);
      }
      return build_token(TOKEN_EQUAL);
    case '<':  // '<='
      if (match('=')) {
        return build_token(TOKEN_LESS_EQUAL);
      }
      return build_token(TOKEN_LESS);
    case '>':  // '>='
      if (match('=')) {
        return build_token(TOKEN_GREATER_EQUAL);
      }
      return build_token(TOKEN_GREATER);
    default:
      return std::nullopt;
  }
}

opt_token_t Lexer::handle_slash_token() {
  if (source_.current() == '/') {
    if (opt_token_t t = try_tokenize_comment()) {
      return *t;
    }
    return build_token(TOKEN_SLASH);
  }
  return std::nullopt;
}

void Lexer::skip_whitespace() {
  while (std::isspace(source_.peek())) {
    advance();
  }
}

Token Lexer::next_token() {
  skip_whitespace();
  current_context_.clear();
  advance();

  std::vector<std::function<opt_token_t()>> token_handlers = {
      [this]() { return handle_single_char_token(); },
      [this]() { return handle_double_char_token(); },
      [this]() { return handle_slash_token(); },
      [this]() { return try_tokenize_string(); },
      [this]() { return try_tokenize_identifier(); },
      [this]() { return try_tokenize_number(); },
  };

  for (auto& handler : token_handlers) {
    if (opt_token_t t = handler()) {
      return *t;
    }
  }

  throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                   "Encountered unknown token");
}

char Lexer::advance() {
  char c = source_.next();
  current_context_ += c;
  return c;
}

bool Lexer::match(char c) {
  if (c == source_.peek()) {
    advance();
    return true;
  }
  return false;
}

Token LexerCommentFilter::next_token() {
  while (true) {
    Token token = lexer_.next_token();
    if (token.get_type() != TOKEN_COMMENT) {
      return token;
    }
  }
}
