#include <iostream>

#include "lexer/lexer.hpp"
#include "source/source.hpp"

int main() {
  auto src = StringSource(L",.{}\"Hello, World!\" hi // a comment\n hello 0.01 123 00");
  auto lexer = Lexer(src);
  while (true) {
    auto token = lexer.next_token();
    std::wcout << token << L' ';
    if (token.type == TokenType::TOKEN_ETX) {
      break;
    }
  }

  return 0;
}
