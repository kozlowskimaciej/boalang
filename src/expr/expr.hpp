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
class NegationExpr;
class LogicalNegationExpr;
class VarExpr;
class LogicalOrExpr;
class LogicalAndExpr;
class IsTypeExpr;
class AsTypeExpr;
class InitalizerListExpr;
class CallExpr;
class FieldAccessExpr;

/**
 * @brief Interface for expressions visitor.
 */
class ExprVisitor {
 public:
  virtual ~ExprVisitor() = default;

  ExprVisitor() = default;
  ExprVisitor(const ExprVisitor&) = delete;
  ExprVisitor& operator=(const ExprVisitor&) = delete;

  ExprVisitor(ExprVisitor&&) = default;
  ExprVisitor& operator=(ExprVisitor&&) = default;

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
  virtual void visit(const NegationExpr& expr) = 0;
  virtual void visit(const LogicalNegationExpr& expr) = 0;
  virtual void visit(const VarExpr& expr) = 0;
  virtual void visit(const LogicalOrExpr& expr) = 0;
  virtual void visit(const LogicalAndExpr& expr) = 0;
  virtual void visit(const IsTypeExpr& expr) = 0;
  virtual void visit(const AsTypeExpr& expr) = 0;
  virtual void visit(const InitalizerListExpr& expr) = 0;
  virtual void visit(const CallExpr& expr) = 0;
  virtual void visit(const FieldAccessExpr& expr) = 0;
};

/**
 * @brief Interface for expressions.
 */
class Expr {
 public:
  Position position;

  virtual ~Expr() = default;
  virtual void accept(ExprVisitor& visitor) const = 0;

  Expr(Position position) : position(position){};
  Expr(const Expr&) = delete;
  Expr& operator=(const Expr&) = delete;

  Expr(Expr&&) = default;
  Expr& operator=(Expr&&) = default;
};

template <typename Derived>
class ExprType : public Expr {
 public:
  using Expr::Expr;
  void accept(ExprVisitor& visitor) const override {
    visitor.visit(static_cast<const Derived&>(*this));
  }
};

template <typename Derived>
class BinaryExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
             Position position)
      : ExprType<Derived>(position),
        left(std::move(left)),
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

template <typename Derived>
class UnaryExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> right;

  UnaryExpr(std::unique_ptr<Expr> right, Position position)
      : ExprType<Derived>(position), right(std::move(right)){};
};

class NegationExpr : public UnaryExpr<NegationExpr> {
 public:
  using UnaryExpr::UnaryExpr;
};

class LogicalNegationExpr : public UnaryExpr<LogicalNegationExpr> {
 public:
  using UnaryExpr::UnaryExpr;
};

template <typename Derived>
class LogicalExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  LogicalExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
              Position position)
      : ExprType<Derived>(position),
        left(std::move(left)),
        right(std::move(right)) {}
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
  value_t literal;

  explicit LiteralExpr(value_t literal, Position position)
      : ExprType(position), literal(std::move(literal)){};
};

class GroupingExpr : public ExprType<GroupingExpr> {
 public:
  std::unique_ptr<Expr> expr;

  explicit GroupingExpr(std::unique_ptr<Expr> expr, Position position)
      : ExprType(position), expr(std::move(expr)){};
};

class VarExpr : public ExprType<VarExpr> {
 public:
  std::string identifier;

  explicit VarExpr(std::string identifier, Position position)
      : ExprType(position), identifier(std::move(identifier)){};
};

template <typename Derived>
class CastExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> left;
  VarType type;

  CastExpr(std::unique_ptr<Expr> left, VarType type, Position position)
      : ExprType<Derived>(position),
        left(std::move(left)),
        type(std::move(type)){};
};

class IsTypeExpr : public CastExpr<IsTypeExpr> {
 public:
  using CastExpr::CastExpr;
};

class AsTypeExpr : public CastExpr<AsTypeExpr> {
 public:
  using CastExpr::CastExpr;
};

class InitalizerListExpr : public ExprType<InitalizerListExpr> {
 public:
  std::vector<std::unique_ptr<Expr>> list;

  explicit InitalizerListExpr(std::vector<std::unique_ptr<Expr>> list,
                              Position position)
      : ExprType(position), list(std::move(list)){};
};

class CallExpr : public ExprType<CallExpr> {
 public:
  std::string identifier;
  std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallExpr(std::string identifier, Position position,
                    std::vector<std::unique_ptr<Expr>> arguments = {})
      : ExprType(position),
        identifier(std::move(identifier)),
        arguments(std::move(arguments)){};
};

class FieldAccessExpr : public ExprType<FieldAccessExpr> {
 public:
  std::unique_ptr<Expr> parent_struct;
  std::string field_name;

  explicit FieldAccessExpr(std::unique_ptr<Expr> parent_struct,
                           std::string field_name, Position position)
      : ExprType(position),
        parent_struct(std::move(parent_struct)),
        field_name(std::move(field_name)){};
};

#endif  // BOALANG_EXPR_HPP
