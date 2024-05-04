/*! @file ast_printer.hpp
    @brief Ast printer.
*/

#ifndef BOALANG_ASTPRINTER_HPP
#define BOALANG_ASTPRINTER_HPP

#include <expr/expr.hpp>
#include <stmt/stmt.hpp>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

class ASTPrinter : public ExprVisitor, public StmtVisitor {
 private:
  unsigned int indent_ = 0;

  void parenthesize(std::initializer_list<std::variant<const Expr*, const Stmt*>> exprstmts,
                    std::optional<Token> token = std::nullopt);
  static void print_memory_info(const std::string& class_name, const void* address);

 public:
  void print(Program* program);

  void visit_program_stmt(const Program& stmt) override;
  void visit_print_stmt(const PrintStmt& stmt) override;
  void visit_if_stmt(const IfStmt& stmt) override;
  void visit_block_stmt(const BlockStmt& stmt) override;
  void visit_while_stmt(const WhileStmt& stmt) override;

  void visit_binary_expr(const BinaryExpr& expr) override;
  void visit_grouping_expr(const GroupingExpr& expr) override;
  void visit_literal_expr(const LiteralExpr& expr) override;
  void visit_unary_expr(const UnaryExpr& expr) override;
  void visit_var_expr(const VarExpr& expr) override;
  void visit_logical_expr(const LogicalExpr& expr) override;
  void visit_cast_expr(const CastExpr& expr) override;
  void visit_type_expr(const TypeExpr& expr) override;
  void visit_initalizerlist_expr(const InitalizerListExpr& expr) override;
  void visit_call_expr(const CallExpr& expr) override;
  void visit_fieldaccess_expr(const FieldAccessExpr& expr) override;
};

#endif  // BOALANG_ASTPRINTER_HPP
