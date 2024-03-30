#include "lexer.hpp"

#include <cmath>

Token Lexer::build_token_with_value(const TokenType& type,
                                    const token_value_t& value) const {
  if (value.has_value()) {
    return {type, value, source_.position()};
  }
  return {type, current_context_, source_.position()};
}

Token Lexer::build_token(const TokenType& type) const {
  return {type, source_.position()};
}

Token Lexer::tokenize_string() {
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

Token Lexer::tokenize_number() {
  int decimal_part = build_decimal();
  if (match('.')) {
    if (!std::iswdigit(source_.peek())) {
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Expected digit after '.'");
    }

    int fraction_part = 0;
    int exponent = 0;
    float float_val;

    while (std::iswdigit(source_.peek())) {
      fraction_part *= 10;
      fraction_part += advance() - '0';
      ++exponent;
      float_val = static_cast<float>(decimal_part +
                                     fraction_part * std::pow(10, -exponent));
    }
    return build_token_with_value(TOKEN_FLOAT_VAL, float_val);
  }
  int int_val = decimal_part;
  return build_token_with_value(TOKEN_INT_VAL, int_val);
}

Token Lexer::tokenize_identifier() {
  while (std::iswalnum(source_.peek()) || source_.peek() == '_') {
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

Token Lexer::tokenize_comment() {
  while (source_.current() != '\n' && !source_.eof()) {
    advance();
  }
  return build_token_with_value(
      TOKEN_COMMENT, current_context_.substr(2, current_context_.length() - 3));
}

Token Lexer::tokenize_long_comment() {
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

int Lexer::build_decimal() {
  if (source_.current() == '0' && match('0')) {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Leading zeros are not allowed");
  }
  int integer = source_.current() - '0';

  int digit;
  while (std::iswdigit(source_.peek())) {
    digit = advance() - '0';
    if (integer > (INT_MAX - digit) / 10) {
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Int literal exceeds maximum value (" +
                           std::to_string(INT_MAX) + ")");
    }
    integer *= 10;
    integer += digit;
  }
  return integer;
}

Token Lexer::next_token() {
  char c;
  do {
    current_context_.clear();
    c = advance();
  } while (std::iswspace(c));

  switch (c) {
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

    // Double chars
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

    case '/':
      if (match('/')) {
        return tokenize_comment();
      }
      if (match('*')) {
        return tokenize_long_comment();
      }
      return build_token(TOKEN_SLASH);

    // Literals
    case '"':
      return tokenize_string();

    default:
      if (std::iswalpha(c) || c == '_') {
        return tokenize_identifier();
      }
      if (std::iswdigit(c)) {
        return tokenize_number();
      }
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Encountered unknown token");
  }
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
