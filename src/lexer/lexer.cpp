#include "lexer.hpp"

Token Lexer::build_token(const TokenType& type,
                         const token_value_t& value) const {
  return {type, value, source_.position()};
}

Token Lexer::tokenize_string() {
  std::wstring str;
  while (source_.peek() != L'"' && source_.peek() != L'\0' &&
         source_.peek() != L'\n') {
    str += source_.next();
  }
  if (source_.peek() == L'\0' || source_.peek() == L'\n') {
      throw LexerError(build_token(TOKEN_UNKNOWN, L'"' + str),
                       L"Unterminated string.");
  }
  source_.next();  // consume closing quote
  return build_token(TOKEN_STR_VAL, str);
}

//Token Lexer::tokenize_number() {
//  return Token(TOKEN_INT_VAL, Position());
//}

Token Lexer::tokenize_identifier() {
  std::wstring str;

  str += source_.current();
  while (std::iswalnum(source_.peek()) || source_.peek() == L'_') {
    str += source_.next();
  }
  if (keywords.find(str) != keywords.end()) {
    return build_token(keywords.at(str));
  }
  if (str.length() > MAX_IDENTIFIER_LENGTH) {
    throw LexerError(build_token(TOKEN_IDENTIFIER, str),
                     L"Identifier exceeds maximum length.");
  }
  return build_token(TOKEN_IDENTIFIER, str);
}

Token Lexer::next_token() {
  const wchar_t c = source_.next();
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
//      if (std::iswdigit(c)) {
//          std::wstring str;
//          str += c;
//          while (std::iswdigit(source_.peek())) {
//          str += source_.next();
//          }
//          if (source_.peek() == L'.' &&
//          std::iswdigit(source_.peek(2))) { str += source_.next();  //
//          consume '.' while (std::iswdigit(source_.peek())) {
//              str += source_.next();
//          }
//          return build_token(TOKEN_FLOATVAL, str);
//          }
//          return build_token(TOKEN_INTVAL, str);
//      }
      throw LexerError(build_token(TOKEN_UNKNOWN, std::wstring{c}),
                       L"Encountered unknown token.");
  }
}
