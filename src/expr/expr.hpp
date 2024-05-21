/*! @file expr.hpp
    @brief Expressions.
*/

#ifndef BOALANG_EXPR_HPP
#define BOALANG_EXPR_HPP

#include <memory>
#include <utility>
#include <vector>

#include "token/token.hpp"

class AdditionExpr;
class SubtractionExpr;
class DivisionExpr;
class MultiplicationExpr;
class EqualCompExpr;
class NotEqualCompExpr;
class GreaterCompExpr;
class GreaterEqualCompExpr;
class LessCompExpr;
class LessEqualCompExpr;
class GroupingExpr;
class LiteralExpr;
class UnaryExpr;
class VarExpr;
class LogicalOrExpr;
class LogicalAndExpr;
class CastExpr;
class InitalizerListExpr;
class CallExpr;
class FieldAccessExpr;

class ExprVisitor {
 public:
  virtual ~ExprVisitor() = default;

  virtual void visit(const AdditionExpr& expr) = 0;
  virtual void visit(const SubtractionExpr& expr) = 0;
  virtual void visit(const DivisionExpr& expr) = 0;
  virtual void visit(const MultiplicationExpr& expr) = 0;
  virtual void visit(const EqualCompExpr& expr) = 0;
  virtual void visit(const NotEqualCompExpr& expr) = 0;
  virtual void visit(const GreaterCompExpr& expr) = 0;
  virtual void visit(const GreaterEqualCompExpr& expr) = 0;
  virtual void visit(const LessCompExpr& expr) = 0;
  virtual void visit(const LessEqualCompExpr& expr) = 0;
  virtual void visit(const GroupingExpr& expr) = 0;
  virtual void visit(const LiteralExpr& expr) = 0;
  virtual void visit(const UnaryExpr& expr) = 0;
  virtual void visit(const VarExpr& expr) = 0;
  virtual void visit(const LogicalOrExpr& expr) = 0;
  virtual void visit(const LogicalAndExpr& expr) = 0;
  virtual void visit(const CastExpr& expr) = 0;
  virtual void visit(const InitalizerListExpr& expr) = 0;
  virtual void visit(const CallExpr& expr) = 0;
  virtual void visit(const FieldAccessExpr& expr) = 0;
};

class Expr {
 public:
  virtual ~Expr() = default;
  virtual void accept(ExprVisitor& visitor) const = 0;
};

template <typename Derived>
class ExprType : public Expr {
 public:
  void accept(ExprVisitor& visitor) const override {
    visitor.visit(static_cast<const Derived&>(*this));
  }
};

template <typename Derived>
class BinaryExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> left;
  Token op_symbol;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> left, Token op_symbol,
             std::unique_ptr<Expr> right)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        right(std::move(right)){};
};

class AdditionExpr : public BinaryExpr<AdditionExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class SubtractionExpr : public BinaryExpr<SubtractionExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class DivisionExpr : public BinaryExpr<DivisionExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class MultiplicationExpr : public BinaryExpr<MultiplicationExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class EqualCompExpr : public BinaryExpr<EqualCompExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class NotEqualCompExpr : public BinaryExpr<NotEqualCompExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class GreaterCompExpr : public BinaryExpr<GreaterCompExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class GreaterEqualCompExpr : public BinaryExpr<GreaterEqualCompExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class LessCompExpr : public BinaryExpr<LessCompExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class LessEqualCompExpr : public BinaryExpr<LessEqualCompExpr> {
 public:
  using BinaryExpr::BinaryExpr;
};

class UnaryExpr : public ExprType<UnaryExpr> {
 public:
  Token op_symbol;
  std::unique_ptr<Expr> right;

  UnaryExpr(Token op_symbol, std::unique_ptr<Expr> right)
      : op_symbol(std::move(op_symbol)), right(std::move(right)){};
};

template <typename Derived>
class LogicalExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> left;
  Token op_symbol;
  std::unique_ptr<Expr> right;

  LogicalExpr(std::unique_ptr<Expr> left, Token op_symbol,
                std::unique_ptr<Expr> right)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        right(std::move(right)){}
};

class LogicalOrExpr : public LogicalExpr<LogicalOrExpr> {
 public:
  using LogicalExpr::LogicalExpr;
};

class LogicalAndExpr : public LogicalExpr<LogicalAndExpr> {
 public:
  using LogicalExpr::LogicalExpr;
};

class LiteralExpr : public ExprType<LiteralExpr> {
 public:
  Token literal;

  explicit LiteralExpr(Token literal) : literal(std::move(literal)){};
};

class GroupingExpr : public ExprType<GroupingExpr> {
 public:
  std::unique_ptr<Expr> expr;

  explicit GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
};

class VarExpr : public ExprType<VarExpr> {
 public:
  Token identifier;

  explicit VarExpr(Token identifier) : identifier(std::move(identifier)){};
};

class CastExpr : public ExprType<CastExpr> {
 public:
  std::unique_ptr<Expr> left;
  Token op_symbol;
  Token type;

  CastExpr(std::unique_ptr<Expr> left, Token op_symbol, Token type)
      : left(std::move(left)),
        op_symbol(std::move(op_symbol)),
        type(std::move(type)){};
};

class InitalizerListExpr : public ExprType<InitalizerListExpr> {
 public:
  std::vector<std::unique_ptr<Expr>> list;

  explicit InitalizerListExpr(std::vector<std::unique_ptr<Expr>> list)
      : list(std::move(list)){};
};

class CallExpr : public ExprType<CallExpr> {
 public:
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallExpr(std::unique_ptr<Expr> callee,
                    std::vector<std::unique_ptr<Expr>> arguments = {})
      : callee(std::move(callee)), arguments(std::move(arguments)){};
};

class FieldAccessExpr : public ExprType<FieldAccessExpr> {
 public:
  std::unique_ptr<Expr> parent_struct;
  Token field_name;

  explicit FieldAccessExpr(std::unique_ptr<Expr> parent_struct,
                           Token field_name)
      : parent_struct(std::move(parent_struct)),
        field_name(std::move(field_name)){};
};

#endif  // BOALANG_EXPR_HPP
