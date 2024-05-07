#include "expr.hpp"

void BinaryExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_binary_expr(*this);
}

void UnaryExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_unary_expr(*this);
}

void LogicalExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_logical_expr(*this);
}

void LiteralExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_literal_expr(*this);
}

void GroupingExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_grouping_expr(*this);
}

void VarExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_var_expr(*this);
}

void CastExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_cast_expr(*this);
}

void InitalizerListExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_initalizerlist_expr(*this);
}

void CallExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_call_expr(*this);
}

void FieldAccessExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit_fieldaccess_expr(*this);
}
