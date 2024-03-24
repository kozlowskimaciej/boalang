#include <iostream>

#include "lexer/lexer.hpp"
#include "source/source.hpp"

int main(int argc, char* argv[]) {
  Source* src;
  if (argc == 2) {
    src = new FileSource(argv[1]);
  } else {
    src = new StringSource(
        L",.{}\"Hello, World!\" hi // a comment\n hello 0.01 123 2147483648");
  }
  auto lexer = Lexer(*src);
  while (true) {
    auto token = lexer.next_token();
    std::wcout << token << L' ';
    if (token.type == TokenType::TOKEN_ETX) {
      break;
    }
  }

  return 0;
}
