/*! @file expr.hpp
    @brief Expressions.
*/

#ifndef BOALANG_EXPR_HPP
#define BOALANG_EXPR_HPP

#include <memory>
#include <utility>

#include "exprvisitor.hpp"
#include "token/token.hpp"


class Expr {
 public:
  virtual ~Expr() = default;
  virtual void accept(ExprVisitor& visitor) const = 0;
};

class AssignExpr : public Expr {
 public:
  const Token name;
  const std::unique_ptr<Expr> value;

  AssignExpr(Token name, std::unique_ptr<Expr> value)
      : name(std::move(name)), value(std::move(value)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_assign_expr(*this);
  };
};

class BinaryExpr : public Expr {
 public:
  const std::unique_ptr<Expr> left;
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> left, Token op_symbol,
             std::unique_ptr<Expr> right)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_binary_expr(*this);
  };
};

class UnaryExpr : public Expr {
 public:
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  UnaryExpr(Token op_symbol, std::unique_ptr<Expr> right)
      : op_symbol(std::move(op_symbol)), right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_unary_expr(*this);
  };
};

class LogicalExpr : public Expr {
 public:
  const std::unique_ptr<Expr> left;
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  LogicalExpr(std::unique_ptr<Expr> left, Token op_symbol,
              std::unique_ptr<Expr> right)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_logical_expr(*this);
  };
};

class LiteralExpr : public Expr {
 public:
//  const token_value_t literal;
  const Token literal;

  explicit LiteralExpr(Token literal) : literal(std::move(literal)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_literal_expr(*this);
  };
};

class GroupingExpr : public Expr {
 public:
  const std::unique_ptr<Expr> expr;

  explicit GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_grouping_expr(*this);
  };
};

class VarExpr : public Expr {
 public:
  const std::string identifier;

  explicit VarExpr(std::string identifier)
      : identifier(std::move(identifier)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_var_expr(*this);
  };
};

#endif  // BOALANG_EXPR_HPP
