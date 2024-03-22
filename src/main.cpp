#include <iostream>

#include "source/source.hpp"
#include "lexer/lexer.hpp"

int main() {
  auto src = StringSource(L",.{}");
  auto lexer = Lexer(src);
  while (true) {
    auto token = lexer.next_token();
    std::wcout << token;
    if (token.type == TokenType::END_OF_FILE) {
      break;
    }
  }

  return 0;
}
