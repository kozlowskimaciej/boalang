#ifndef BOALANG_LEXER_HPP
#define BOALANG_LEXER_HPP

#include <vector>

#include "../source/source.hpp"
#include "../token/token.hpp"

class Lexer {
 private:
  const Source& source_;

 public:
  explicit Lexer(const Source& source) : source_(source){};
  Token next_token();
};

#endif  // BOALANG_LEXER_HPP
