/*! @file parser.hpp
    @brief Parser.
*/

#ifndef BOALANG_PARSER_HPP
#define BOALANG_PARSER_HPP

#include <expr/expr.hpp>
#include <lexer/lexer.hpp>
#include <memory>
#include <vector>

class Parser {
  ILexer& lexer_;
  Token current_token_;

  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> assignment();
  std::unique_ptr<Expr> logic_or();
  std::unique_ptr<Expr> logic_and();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  //  std::unique_ptr<Expr> type_cast();
  //  std::unique_ptr<Expr> call();
  std::unique_ptr<Expr> primary();

  opt_token_t match(std::initializer_list<TokenType> types);
  Token advance();
  Token consume(std::initializer_list<TokenType> types,
                const std::string& err_msg);

 public:
  explicit Parser(ILexer& lexer)
      : lexer_(lexer), current_token_(lexer.next_token()){};
  std::vector<std::unique_ptr<Expr>> parse();
};

/**
 * @brief Represents a parser related error.
 */
class ParserError : public std::runtime_error {
  Token token_;

 public:
  ParserError(const Token& token, const std::string& message)
      : runtime_error("SyntaxError: Line " +
                      std::to_string(token.get_position().line) + " column " +
                      std::to_string(token.get_position().column) + " at '" +
                      token.stringify() + "': " + message),
        token_(token){};

  [[nodiscard]] const Token& get_token() const { return token_; }
};

#endif  // BOALANG_PARSER_HPP