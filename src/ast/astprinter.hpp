/*! @file ast_printer.hpp
    @brief Ast printer.
*/

#ifndef BOALANG_ASTPRINTER_HPP
#define BOALANG_ASTPRINTER_HPP

#include <expr/expr.hpp>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>

class ASTPrinter : public ExprVisitor {
 private:
  unsigned int indent_ = 0;
  std::stringstream stream_;

  void parenthesize(std::initializer_list<const Expr*> exprs,
                    std::optional<Token> token = std::nullopt);
  void print_memory_info(const std::string& class_name, const void* address);

 public:
  void print(const std::vector<std::unique_ptr<Expr>>& exprs);
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
