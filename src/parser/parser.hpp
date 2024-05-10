/*! @file parser.hpp
    @brief Parser.
*/

#ifndef BOALANG_PARSER_HPP
#define BOALANG_PARSER_HPP

#include <expr/expr.hpp>
#include <lexer/lexer.hpp>
#include <memory>
#include <stmt/stmt.hpp>
#include <vector>

class Parser {
  ILexer& lexer_;
  Token current_token_;

  std::unique_ptr<Stmt> statement();
  std::unique_ptr<IfStmt> if_stmt();
  std::unique_ptr<WhileStmt> while_stmt();
  std::unique_ptr<ReturnStmt> return_stmt();
  std::unique_ptr<PrintStmt> print_stmt();
  std::unique_ptr<InspectStmt> inspect_stmt();
  std::unique_ptr<LambdaFuncStmt> lambda_func();
  std::unique_ptr<BlockStmt> block_stmt();
  std::unique_ptr<Stmt> declaration();

  std::unique_ptr<Stmt> assign_call_decl();
  std::unique_ptr<VarDeclStmt> mut_var_decl();
  std::unique_ptr<FuncStmt> void_func_decl();
  std::unique_ptr<Stmt> var_func_decl(const Token& type);
  std::unique_ptr<VarDeclStmt> var_decl(const Token& type,
                                        const Token& identifier, bool mut);
  std::unique_ptr<FuncStmt> func_decl(const Token& return_type,
                                      const Token& identifier);
  std::optional<std::vector<std::unique_ptr<FuncParamStmt>>> func_params();
  std::unique_ptr<Stmt> assign_call(const Token& identifier);
  std::unique_ptr<AssignStmt> assign_stmt(std::unique_ptr<Expr> var);
  std::unique_ptr<CallStmt> call_stmt(const Token& identifier);
  std::unique_ptr<StructDeclStmt> struct_decl();
  std::unique_ptr<StructFieldStmt> struct_field();
  std::unique_ptr<VariantDeclStmt> variant_decl();
  std::optional<std::vector<Token>> variant_params();

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

  opt_token_t match(std::initializer_list<TokenType> types);
  Token advance();
  Token consume(std::initializer_list<TokenType> types,
                const std::string& err_msg);
  [[nodiscard]] bool check(std::initializer_list<TokenType> types) const;

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
