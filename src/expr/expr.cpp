#include "expr.hpp"

void BinaryExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void UnaryExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void LogicalExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void LiteralExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void GroupingExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void VarExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void CastExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void InitalizerListExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void CallExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}

void FieldAccessExpr::accept(ExprVisitor &expr_visitor) const {
  expr_visitor.visit(*this);
}
