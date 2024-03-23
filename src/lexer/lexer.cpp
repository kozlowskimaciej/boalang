#include <cfloat>
#include "lexer.hpp"

Token Lexer::build_token(const TokenType& type,
                         const token_value_t& value) const {
  return {type, value, source_.position()};
}

Token Lexer::tokenize_string() {
  while (source_.peek() != L'"' && !source_.eof() &&
         source_.peek() != L'\n') {
    advance();
  }
  if (source_.peek() != L'"') {
      throw LexerError(build_token(TOKEN_UNKNOWN, current_scope_),
                       L"Unterminated string");
  }
  advance();  // consume closing quote
  return build_token(TOKEN_STR_VAL, current_scope_.substr(1, current_scope_.length() - 2));
}

Token Lexer::tokenize_number() {
  if (source_.current() == L'0' && source_.peek() == L'0') {
    throw LexerError(build_token(TOKEN_INT_VAL, current_scope_),
                     L"Leading zeros are not allowed");
  }

  while (std::iswdigit(source_.peek())) {
    advance();
  }
  if (source_.peek() == L'.') {
    advance();
    if (!std::iswdigit(source_.peek())) {
        throw LexerError(build_token(TOKEN_FLOAT_VAL, current_scope_),
                         L"Expected digit after '.'");
    }
    while (std::iswdigit(source_.peek())) {
      advance();
    }

    float val;
    try {
      val = std::stof(current_scope_);
    } catch (std::out_of_range&) {
      throw LexerError(build_token(TOKEN_FLOAT_VAL, current_scope_),
                       L"Float literal exceeds maximum value");
    }
    return build_token(TOKEN_FLOAT_VAL, val);
  }

  int val;
  try {
    val = std::stoi(current_scope_);
  } catch (std::out_of_range&) {
    throw LexerError(build_token(TOKEN_INT_VAL, current_scope_),
                     L"Int literal exceeds maximum value");
  }
  return build_token(TOKEN_INT_VAL, val);
}

Token Lexer::tokenize_identifier() {
  while (std::iswalnum(source_.peek()) || source_.peek() == L'_') {
    advance();
  }
  if (keywords.find(current_scope_) != keywords.end()) {
    return build_token(keywords.at(current_scope_));
  }
  if (current_scope_.length() > MAX_IDENTIFIER_LENGTH) {
    throw LexerError(build_token(TOKEN_IDENTIFIER, current_scope_),
                     L"Identifier exceeds maximum length");
  }
  return build_token(TOKEN_IDENTIFIER, current_scope_);
}

Token Lexer::next_token() {
  current_scope_ = L"";
  wchar_t c = advance();

  switch (c) {
    case L'\0':
      return build_token(TOKEN_ETX);
    case L'(':
      return build_token(TOKEN_LPAREN);
    case L')':
      return build_token(TOKEN_RPAREN);
    case L'{':
      return build_token(TOKEN_LBRACE);
    case L'}':
      return build_token(TOKEN_RBRACE);
    case L',':
      return build_token(TOKEN_COMMA);
    case L'.':
      return build_token(TOKEN_DOT);
    case L'-':
      return build_token(TOKEN_MINUS);
    case L'+':
      return build_token(TOKEN_PLUS);
    case L';':
      return build_token(TOKEN_SEMICOLON);
    case L'/':
      if (source_.peek() == L'/') {
        while (source_.peek() != L'\n' && !source_.eof()) {
          advance();
        }
        return next_token();
      }
      return build_token(TOKEN_SLASH);
    case L'*':
      return build_token(TOKEN_STAR);

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

    // Literals
    case L'"':
      return tokenize_string();

    default:
      if (std::iswalpha(c) || c == L'_') {
          return tokenize_identifier();
      }
      if (std::iswdigit(c)) {
          return tokenize_number();
      }
      throw LexerError(build_token(TOKEN_UNKNOWN, current_scope_),
                       L"Encountered unknown token");
  }
}

wchar_t Lexer::advance() {
  wchar_t c = source_.next();
  while (std::iswspace(c)) {
    c = source_.next();
  }
  current_scope_ += c;
  return c;
}
