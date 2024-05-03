/*! @file expr.hpp
    @brief Expressions.
*/

#ifndef BOALANG_EXPR_HPP
#define BOALANG_EXPR_HPP

#include <memory>
#include <utility>
#include <vector>

#include "exprvisitor.hpp"
#include "token/token.hpp"

class Expr {
 public:
  virtual ~Expr() = default;
  virtual void accept(ExprVisitor& visitor) const = 0;
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

class CastExpr : public Expr {
 public:
  const std::unique_ptr<Expr> left;
  const Token op_symbol;
  const std::unique_ptr<Expr> type;

  CastExpr(std::unique_ptr<Expr> left, Token op_symbol,
           std::unique_ptr<Expr> type)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        type(std::move(type)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_cast_expr(*this);
  };
};

class TypeExpr : public Expr {
 public:
  const Token type;

  explicit TypeExpr(Token type) : type(std::move(type)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_type_expr(*this);
  };
};

class InitalizerListExpr : public Expr {
 public:
  const std::vector<std::unique_ptr<Expr>> list;

  explicit InitalizerListExpr(std::vector<std::unique_ptr<Expr>> list)
      : list(std::move(list)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_initalizerlist_expr(*this);
  };
};

class CallExpr : public Expr {
 public:
  const std::unique_ptr<Expr> callee;
  const std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallExpr(std::unique_ptr<Expr> callee,
                    std::vector<std::unique_ptr<Expr>> arguments)
      : callee(std::move(callee)), arguments(std::move(arguments)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_call_expr(*this);
  };
};

class FieldAccessExpr : public Expr {
 public:
  const std::unique_ptr<Expr> parent_struct;
  const Token field_name;

  explicit FieldAccessExpr(std::unique_ptr<Expr> parent_struct,
                           Token field_name)
      : parent_struct(std::move(parent_struct)),
        field_name(std::move(field_name)){};
  void accept(ExprVisitor& expr_visitor) const override {
    expr_visitor.visit_fieldaccess_expr(*this);
  };
};

#endif  // BOALANG_EXPR_HPP
