/*! @file exprvisitor.hpp
    @brief Expression visitor.
*/

#ifndef BOALANG_EXPRVISITOR_HPP
#define BOALANG_EXPRVISITOR_HPP

// forward declarations
class BinaryExpr;
class GroupingExpr;
class LiteralExpr;
class UnaryExpr;
class VarExpr;
class LogicalExpr;
class AssignExpr;
class CastExpr;
class TypeExpr;
class InitalizerListExpr;

class ExprVisitor {
 public:
  virtual ~ExprVisitor() = default;

  virtual void visit_binary_expr(const BinaryExpr& expr) = 0;
  virtual void visit_grouping_expr(const GroupingExpr& expr) = 0;
  virtual void visit_literal_expr(const LiteralExpr& expr) = 0;
  virtual void visit_unary_expr(const UnaryExpr& expr) = 0;
  virtual void visit_var_expr(const VarExpr& expr) = 0;
  virtual void visit_logical_expr(const LogicalExpr& expr) = 0;
  virtual void visit_assign_expr(const AssignExpr& expr) = 0;
  virtual void visit_cast_expr(const CastExpr& expr) = 0;
  virtual void visit_type_expr(const TypeExpr& expr) = 0;
  virtual void visit_initalizerlist_expr(const InitalizerListExpr& expr) = 0;
};

#endif  // BOALANG_EXPRVISITOR_HPP
