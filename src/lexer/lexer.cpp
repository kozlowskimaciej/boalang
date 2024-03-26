#include <cmath>
#include "lexer.hpp"

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
  while (source_.peek() != L'"' && !source_.eof() && source_.peek() != L'\n') {
    advance();
  }
  if (source_.peek() != L'"') {
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
  if (source_.current() == L'0' && source_.peek() == L'0') {
    throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                     "Leading zeros are not allowed");
  }

  int decimal_part = source_.current() - L'0';

  int digit;
  while (std::iswdigit(source_.peek())) {
    digit = advance() - L'0';
    if (decimal_part > (INT_MAX - digit) / 10) {
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Int literal exceeds maximum value (" + std::to_string(INT_MAX) + ")");
    }
    decimal_part *= 10;
    decimal_part += digit;
  }
  if (source_.peek() == L'.') {
    advance();
    if (!std::iswdigit(source_.peek())) {
      throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                       "Expected digit after '.'");
    }

    int fraction_part = 0;
    int exponent = 0;
    float float_val;

    while (std::iswdigit(source_.peek())) {
      fraction_part *= 10;
      fraction_part += advance() - L'0';
      ++exponent;
      float_val = static_cast<float>(decimal_part + fraction_part*std::pow(10, -exponent));
      if (float_val == HUGE_VALF) {
        throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                         "Float literal exceeds maximum value");
      }
    }
    return build_token_with_value(TOKEN_FLOAT_VAL, float_val);
  }
  int int_val = decimal_part;
  return build_token_with_value(TOKEN_INT_VAL, int_val);
}

Token Lexer::tokenize_identifier() {
  while (std::iswalnum(source_.peek()) || source_.peek() == L'_') {
    advance();
  }
  if (keywords.find(current_context_) != keywords.end()) {
    return build_token(keywords.at(current_context_));
  }
  if (current_context_.length() > MAX_IDENTIFIER_LENGTH) {
    throw LexerError(build_token_with_value(TOKEN_IDENTIFIER),
                     "Identifier exceeds maximum length");
  }
  return build_token_with_value(TOKEN_IDENTIFIER);
}

Token Lexer::tokenize_comment() {
  while (source_.current() != L'\n' && !source_.eof()) {
    advance();
  }
  return build_token_with_value(TOKEN_COMMENT, current_context_.substr(2, current_context_.length() - 3));
}

Token Lexer::tokenize_long_comment() {
  while (!source_.eof()) {
    if (advance() == L'*' && source_.peek() == L'/') {
      advance();  // consume closing comment
      return build_token_with_value(TOKEN_COMMENT, current_context_.substr(2, current_context_.length() - 4));
    }
  }
  throw LexerError(build_token_with_value(TOKEN_UNKNOWN),
                   "Unterminated long comment");
}


Token Lexer::next_token() {
  char c;
  do {
    current_context_.clear();
    c = advance();
  } while(std::iswspace(c));

  switch (c) {
    case L'\0':return build_token(TOKEN_ETX);
    case L'(':return build_token(TOKEN_LPAREN);
    case L')':return build_token(TOKEN_RPAREN);
    case L'{':return build_token(TOKEN_LBRACE);
    case L'}':return build_token(TOKEN_RBRACE);
    case L',':return build_token(TOKEN_COMMA);
    case L'.':return build_token(TOKEN_DOT);
    case L'-':return build_token(TOKEN_MINUS);
    case L'+':return build_token(TOKEN_PLUS);
    case L';':return build_token(TOKEN_SEMICOLON);
    case L'*':return build_token(TOKEN_STAR);

    // Double chars
    case L'!':  // '!='
      if (source_.peek() == L'=') {
        return build_token(TOKEN_NOT_EQUAL);
      }
      return build_token(TOKEN_EXCLAMATION);
    case L'=':  // '=='
      if (source_.peek() == L'=') {
        return build_token(TOKEN_EQUAL_EQUAL);
      }
      if (source_.peek() == L'>') {
        return build_token(TOKEN_ARROW);
      }
      return build_token(TOKEN_EQUAL);
    case L'<':  // '<='
      if (source_.peek() == L'=') {
        return build_token(TOKEN_LESS_EQUAL);
      }
      return build_token(TOKEN_LESS);
    case L'>':  // '>='
      if (source_.peek() == L'=') {
        return build_token(TOKEN_GREATER_EQUAL);
      }
      return build_token(TOKEN_GREATER);

    case L'/':
      if (source_.peek() == L'/') {
        return tokenize_comment();
      }
      if (source_.peek() == L'*') {
        return tokenize_long_comment();
      }
      return build_token(TOKEN_SLASH);

    // Literals
    case L'"':return tokenize_string();

    default:
      if (std::iswalpha(c) || c == L'_') {
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
