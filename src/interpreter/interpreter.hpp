/*! @file interpreter.hpp
    @brief boalang interpreter.
*/

#ifndef BOALANG_INTERPRETER_HPP
#define BOALANG_INTERPRETER_HPP

#include <optional>

#include "expr/expr.hpp"
#include "stmt/stmt.hpp"
#include "interpreter/scope/scope.hpp"
#include "utils/errors.hpp"

class Interpreter : public ExprVisitor, public StmtVisitor {
  std::optional<eval_value_t> evaluation = std::nullopt;
  eval_value_t evaluate(const Expr* expr);
  void set_evaluation(eval_value_t value);
  eval_value_t get_evaluation();

 public:
  void visit(const Program& stmt) override;
  void visit(const PrintStmt& stmt) override;
  void visit(const IfStmt& stmt) override;
  void visit(const BlockStmt& stmt) override;
  void visit(const WhileStmt& stmt) override;
  void visit(const VarDeclStmt& stmt) override;
  void visit(const StructFieldStmt& stmt) override;
  void visit(const StructDeclStmt& stmt) override;
  void visit(const VariantDeclStmt& stmt) override;
  void visit(const AssignStmt& stmt) override;
  void visit(const CallStmt& stmt) override;
  void visit(const FuncParamStmt& stmt) override;
  void visit(const FuncStmt& stmt) override;
  void visit(const ReturnStmt& stmt) override;
  void visit(const LambdaFuncStmt& stmt) override;
  void visit(const InspectStmt& stmt) override;

  void visit(const AdditionExpr& expr) override;
  void visit(const SubtractionExpr& expr) override;
  void visit(const DivisionExpr& expr) override;
  void visit(const MultiplicationExpr& expr) override;
  void visit(const EqualCompExpr& expr) override;
  void visit(const NotEqualCompExpr& expr) override;
  void visit(const GreaterCompExpr& expr) override;
  void visit(const GreaterEqualCompExpr& expr) override;
  void visit(const LessCompExpr& expr) override;
  void visit(const LessEqualCompExpr& expr) override;
  void visit(const GroupingExpr& expr) override;
  void visit(const LiteralExpr& expr) override;
  void visit(const NegationExpr& expr) override;
  void visit(const LogicalNegationExpr& expr) override;
  void visit(const VarExpr& expr) override;
  void visit(const LogicalOrExpr& expr) override;
  void visit(const LogicalAndExpr& expr) override;
  void visit(const IsTypeExpr& expr) override;
  void visit(const AsTypeExpr& expr) override;
  void visit(const InitalizerListExpr& expr) override;
  void visit(const CallExpr& expr) override;
  void visit(const FieldAccessExpr& expr) override;
};

#endif  // BOALANG_INTERPRETER_HPP
