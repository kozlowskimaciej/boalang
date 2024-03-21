#include "lexer.hpp"

#include <utility>

Token Lexer::build_token(const TokenType& type, const token_value_t& value) const {
  return {type, value,source_.position()};
}


Token Lexer::tokenize_string() {
  std::wstring str;
  while (source_.peek() != L'"' && source_.peek() != L'\0' && source_.peek() != L'\n') {
    str += source_.next();
  }
  source_.next();  // consume closing quote
  return build_token(TOKEN_STRVAL, str);
}


Token Lexer::next_token() {
  wchar_t c = source_.next();
  switch(c) {
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
    case L'"': {
        return tokenize_string();
    }
    default:

  }
}
