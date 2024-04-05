#include "lexer.hpp"

#include <cmath>
#include <map>

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

Token Lexer::build_token_with_value(const TokenType& type,
                                    const token_value_t& value) const {
  if (!std::holds_alternative<std::monostate>(value)) {
    return {type, value, source_.position()};
  }
  return {type, current_context_, source_.position()};
}

Token Lexer::build_token(const TokenType& type) const {
  return {type, source_.position()};
}

opt_token_t Lexer::try_tokenize_string() {
  if (source_.current() != '"') { return std::nullopt; }

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
  if (!std::isdigit(c)) { return std::nullopt; }

  if (c == '0' && match('0')) {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Leading zeros are not allowed");
  }
  int value = c - '0';
  bool fraction_part = false;
  int exponent = 0;

  while (std::isdigit(source_.peek()) || source_.peek() == '.') {
    if (source_.peek() == '.') {
      if (fraction_part) {
        throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                         "Expected digit after '.'");
      }
      fraction_part = true;
      advance();
      continue;
    }
    int digit = advance() - '0';
    if (value > (INT_MAX - digit) / 10) {
      if (fraction_part) {
        throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                         "Float literal exceeds range (" +
                             std::to_string(INT_MAX) + ".0, 0." +
                             std::to_string(INT_MAX) + ")");
      }
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Int literal exceeds maximum value (" +
                           std::to_string(INT_MAX) + ")");
    }
    value *= 10;
    value += digit;
    if (fraction_part) {
      ++exponent;
      if (exponent > 10) {
        throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                         "Float literal exceeds range (" +
                             std::to_string(INT_MAX) + ".0, 0." +
                             std::to_string(INT_MAX) + ")");
      }
    }
  }

  if (fraction_part) {
    return build_token_with_value(
        TOKEN_FLOAT_VAL, static_cast<float>(value * std::pow(10, -exponent)));
  }
  return build_token_with_value(TOKEN_INT_VAL, value);
}

opt_token_t Lexer::try_tokenize_identifier() {
  if (!std::isalpha(source_.current()) && source_.current() != '_') {
    return std::nullopt;
  }

  while (std::isalnum(source_.peek()) || source_.peek() == '_') {
    advance();
  }
  if (keywords.find(current_context_) != keywords.end()) {
    return build_token(keywords.at(current_context_));
  }
  if (current_context_.length() > MAX_IDENTIFIER_LENGTH) {
    throw LexerError(build_token_with_value(TOKEN_IDENTIFIER),
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
        TOKEN_COMMENT, current_context_.substr(2, current_context_.length() - 3));
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

  if (opt_token_t t = handle_single_char_token()) {
    return *t;
  }
  if (opt_token_t t = handle_double_char_token()) {
    return *t;
  }
  if (opt_token_t t = handle_slash_token()) {
    return *t;
  }
  if (opt_token_t t = try_tokenize_string()) {
    return *t;
  }
  if (opt_token_t t = try_tokenize_identifier()) {
    return *t;
  }
  if (opt_token_t t = try_tokenize_number()) {
    return *t;
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

bool Lexer::is_exhausted() const { return source_.eof(); }

Token FilteredLexer::next_token() {
  while (true) {
    Token token = Lexer::next_token();
    if (token.type != TOKEN_COMMENT) {
      return token;
    }
  }
}
