#include <iostream>

#include "token/token.hpp"

int main() {
  Token t = Token(TOKEN_INTVAL, 10, {1, 1});
  Token t2 = Token(TOKEN_INT, {1, 1});

  std::cout << (t2.value == std::nullopt ? "true" : "false");

  return 0;
}
