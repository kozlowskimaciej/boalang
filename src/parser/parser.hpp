/*! @file parser.hpp
    @brief Parser.
*/

#ifndef BOALANG_PARSER_HPP
#define BOALANG_PARSER_HPP

#include <expr/expr.hpp>
#include <stmt/stmt.hpp>
#include <lexer/lexer.hpp>
#include <memory>
#include <vector>

constexpr unsigned int MAX_ARGUMENTS =
    256; /**< Maximum identifier length supported by lexer. */

class Parser {
  ILexer& lexer_;
  Token current_token_;

  std::unique_ptr<Stmt> declaration();
  std::unique_ptr<Stmt> assign_or_decl();
  std::unique_ptr<Stmt> var_decl();
  std::unique_ptr<Stmt> statement();
  std::unique_ptr<PrintStmt> print_stmt();
  std::unique_ptr<IfStmt> if_stmt();
  std::unique_ptr<BlockStmt> block_stmt();
  std::unique_ptr<WhileStmt> while_stmt();

  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> logic_or();
  std::unique_ptr<Expr> logic_and();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> type_cast();
  std::unique_ptr<Expr> call();
  std::unique_ptr<Expr> primary();

  std::vector<std::unique_ptr<Expr>> arguments();
  std::unique_ptr<Expr> field_access(std::unique_ptr<Expr> parent_struct);
  std::unique_ptr<Expr> type();

  opt_token_t match(std::initializer_list<TokenType> types);
  Token advance();
  Token consume(std::initializer_list<TokenType> types,
                const std::string& err_msg);
  bool check(std::initializer_list<TokenType> types);

 public:
  explicit Parser(ILexer& lexer)
      : lexer_(lexer), current_token_(lexer.next_token()){};
  std::unique_ptr<Program> parse();
};

/**
 * @brief Represents a parser related error.
 */
class SyntaxError : public std::runtime_error {
  Token token_;

 public:
  SyntaxError(const Token& token, const std::string& message)
      : runtime_error("Line " + std::to_string(token.get_position().line) +
                      " column " + std::to_string(token.get_position().column) +
                      " at '" + token.stringify() + "': " + message),
        token_(token){};

  [[nodiscard]] const Token& get_token() const { return token_; }
};

#endif  // BOALANG_PARSER_HPP
