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
  std::unique_ptr<Expr> left;
  Token op_symbol;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> left, Token op_symbol,
             std::unique_ptr<Expr> right)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class UnaryExpr : public Expr {
 public:
  Token op_symbol;
  std::unique_ptr<Expr> right;

  UnaryExpr(Token op_symbol, std::unique_ptr<Expr> right)
      : op_symbol(std::move(op_symbol)), right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class LogicalExpr : public Expr {
 public:
  std::unique_ptr<Expr> left;
  Token op_symbol;
  std::unique_ptr<Expr> right;

  LogicalExpr(std::unique_ptr<Expr> left, Token op_symbol,
              std::unique_ptr<Expr> right)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        right(std::move(right)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class LiteralExpr : public Expr {
 public:
  Token literal;

  explicit LiteralExpr(Token literal) : literal(std::move(literal)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class GroupingExpr : public Expr {
 public:
  std::unique_ptr<Expr> expr;

  explicit GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class VarExpr : public Expr {
 public:
  Token identifier;

  explicit VarExpr(Token identifier) : identifier(std::move(identifier)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class CastExpr : public Expr {
 public:
  std::unique_ptr<Expr> left;
  Token op_symbol;
  Token type;

  CastExpr(std::unique_ptr<Expr> left, Token op_symbol, Token type)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        type(std::move(type)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class InitalizerListExpr : public Expr {
 public:
  std::vector<std::unique_ptr<Expr>> list;

  explicit InitalizerListExpr(std::vector<std::unique_ptr<Expr>> list)
      : list(std::move(list)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class CallExpr : public Expr {
 public:
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallExpr(std::unique_ptr<Expr> callee,
                    std::vector<std::unique_ptr<Expr>> arguments = {})
      : callee(std::move(callee)), arguments(std::move(arguments)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class FieldAccessExpr : public Expr {
 public:
  std::unique_ptr<Expr> parent_struct;
  Token field_name;

  explicit FieldAccessExpr(std::unique_ptr<Expr> parent_struct,
                           Token field_name)
      : parent_struct(std::move(parent_struct)),
        field_name(std::move(field_name)){};
  void accept(ExprVisitor& expr_visitor) const override;
};

class ExprVisitor {
 public:
  virtual ~ExprVisitor() = default;

  virtual void visit(const BinaryExpr& expr) = 0;
  virtual void visit(const GroupingExpr& expr) = 0;
  virtual void visit(const LiteralExpr& expr) = 0;
  virtual void visit(const UnaryExpr& expr) = 0;
  virtual void visit(const VarExpr& expr) = 0;
  virtual void visit(const LogicalExpr& expr) = 0;
  virtual void visit(const CastExpr& expr) = 0;
  virtual void visit(const InitalizerListExpr& expr) = 0;
  virtual void visit(const CallExpr& expr) = 0;
  virtual void visit(const FieldAccessExpr& expr) = 0;
};

#endif  // BOALANG_EXPR_HPP
