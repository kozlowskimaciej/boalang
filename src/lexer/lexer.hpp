#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include <vector>

#include "token/token.hpp"
#include "utils/position.hpp"

class Lexer {
 private:
  const std::string source;
  std::vector<Token> tokens;
  unsigned int start = 0;
  unsigned int current = 0;
  Position position = {1, 0};

  char advance();
  void tokenize_string();
  void tokenize_number();
  void tokenize_identifier();
  void increase_line_no();
  void add_token(TokenType type);
  void add_token(TokenType type, std::string text);
  void scan_token();

 public:
  Lexer(std::string source) : source(source){};
  std::vector<Token>& scan_tokens();
};

#endif  // BOALANG_LEXER_HPP
