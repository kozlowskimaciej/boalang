/*! @file expr.hpp
    @brief Expressions.
*/

#ifndef BOALANG_EXPR_HPP
#define BOALANG_EXPR_HPP

#include <memory>
#include <utility>
#include <vector>

#include "token/token.hpp"

class ExprVisitor;

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
  void accept(ExprVisitor& expr_visitor) const override;
};

class UnaryExpr : public Expr {
 public:
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  UnaryExpr(Token op_symbol, std::unique_ptr<Expr> right)
      : op_symbol(std::move(op_symbol)), right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override;
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
  void accept(ExprVisitor& expr_visitor) const override;
};

class LiteralExpr : public Expr {
 public:
  const Token literal;

  explicit LiteralExpr(Token literal) : literal(std::move(literal)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class GroupingExpr : public Expr {
 public:
  const std::unique_ptr<Expr> expr;

  explicit GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class VarExpr : public Expr {
 public:
  const Token identifier;

  explicit VarExpr(Token identifier) : identifier(std::move(identifier)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class CastExpr : public Expr {
 public:
  const std::unique_ptr<Expr> left;
  const Token op_symbol;
  const Token type;

  CastExpr(std::unique_ptr<Expr> left, Token op_symbol, Token type)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        type(std::move(type)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class InitalizerListExpr : public Expr {
 public:
  const std::vector<std::unique_ptr<Expr>> list;

  explicit InitalizerListExpr(std::vector<std::unique_ptr<Expr>> list)
      : list(std::move(list)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class CallExpr : public Expr {
 public:
  const std::unique_ptr<Expr> callee;
  const std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallExpr(std::unique_ptr<Expr> callee,
                    std::vector<std::unique_ptr<Expr>> arguments = {})
      : callee(std::move(callee)), arguments(std::move(arguments)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class FieldAccessExpr : public Expr {
 public:
  const std::unique_ptr<Expr> parent_struct;
  const Token field_name;

  explicit FieldAccessExpr(std::unique_ptr<Expr> parent_struct,
                           Token field_name)
      : parent_struct(std::move(parent_struct)),
        field_name(std::move(field_name)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class ExprVisitor {
 public:
  virtual ~ExprVisitor() = default;

  virtual void visit_binary_expr(const BinaryExpr& expr) = 0;
  virtual void visit_grouping_expr(const GroupingExpr& expr) = 0;
  virtual void visit_literal_expr(const LiteralExpr& expr) = 0;
  virtual void visit_unary_expr(const UnaryExpr& expr) = 0;
  virtual void visit_var_expr(const VarExpr& expr) = 0;
  virtual void visit_logical_expr(const LogicalExpr& expr) = 0;
  virtual void visit_cast_expr(const CastExpr& expr) = 0;
  virtual void visit_initalizerlist_expr(const InitalizerListExpr& expr) = 0;
  virtual void visit_call_expr(const CallExpr& expr) = 0;
  virtual void visit_fieldaccess_expr(const FieldAccessExpr& expr) = 0;
};

#endif  // BOALANG_EXPR_HPP
