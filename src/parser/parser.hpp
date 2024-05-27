/*! @file parser.hpp
    @brief Parser.
*/

#ifndef BOALANG_PARSER_HPP
#define BOALANG_PARSER_HPP

#include <expr/expr.hpp>
#include <functional>
#include <lexer/lexer.hpp>
#include <memory>
#include <stmt/stmt.hpp>
#include <type_traits>
#include <vector>

class Parser {
  ILexer& lexer_;
  Token current_token_;

  using StmtHandlers =
      std::initializer_list<std::unique_ptr<Stmt> (Parser::*)()>;
  static const StmtHandlers stmt_handlers;
  static const StmtHandlers declaration_handlers;
  std::unique_ptr<Stmt> try_handlers(const StmtHandlers handlers);

  std::unique_ptr<Stmt> statement();
  std::unique_ptr<Stmt> if_stmt();
  std::unique_ptr<Stmt> while_stmt();
  std::unique_ptr<Stmt> return_stmt();
  std::unique_ptr<Stmt> print_stmt();
  std::unique_ptr<Stmt> inspect_stmt();
  std::unique_ptr<LambdaFuncStmt> lambda_func();
  std::unique_ptr<Stmt> block_stmt();
  std::unique_ptr<Stmt> var_or_func();
  std::unique_ptr<Stmt> struct_decl();
  std::unique_ptr<StructFieldStmt> struct_field();
  std::unique_ptr<Stmt> variant_decl();
  std::optional<std::vector<VarType>> variant_params();

  std::unique_ptr<Stmt> mut_var_decl();
  std::unique_ptr<Stmt> void_func_decl();
  std::unique_ptr<Stmt> var_or_func_decl(const Token& type);
  std::unique_ptr<VarDeclStmt> var_decl(const Token& type,
                                        const Token& identifier, bool mut);
  std::unique_ptr<FuncStmt> func_decl(const Token& return_type,
                                      const Token& identifier);
  std::optional<std::vector<std::unique_ptr<FuncParamStmt>>> func_params();
  std::unique_ptr<Stmt> assign_or_call(const Token& identifier);
  std::unique_ptr<AssignStmt> assign_stmt(std::unique_ptr<Expr> var);
  std::unique_ptr<CallStmt> call_stmt(const Token& identifier);

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

  std::optional<std::vector<std::unique_ptr<Expr>>> arguments();
  std::unique_ptr<Expr> field_access(std::unique_ptr<Expr> parent_struct);
  std::optional<Token> type();

  template <typename... TokenTypes>
  opt_token_t match(TokenTypes&&... types);

  Token advance();

  template <typename... TokenTypes>
  std::enable_if_t<(std::is_same_v<TokenTypes, TokenType> && ...), Token>
  consume(const std::string& err_msg, TokenTypes... types);

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
                      " at '" +
                      (token.stringify().empty() ? token.stringify_type()
                                                 : token.stringify()) +
                      "': " + message),
        token_(token){};

  [[nodiscard]] const Token& get_token() const { return token_; }
};

#endif  // BOALANG_PARSER_HPP
