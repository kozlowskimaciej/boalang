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
  std::unique_ptr<Expr> right;
  Position position;

  BinaryExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
             Position position)
      : left(std::move(left)), right(std::move(right)), position(position){};
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
  Position position;

  UnaryExpr(std::unique_ptr<Expr> right, Position position)
      : right(std::move(right)), position(position){};
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
  Position position;

  LogicalExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
              Position position)
      : left(std::move(left)), right(std::move(right)), position(position) {}
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
  Position position;

  explicit LiteralExpr(value_t literal, Position position)
      : literal(std::move(literal)), position(position){};
};

class GroupingExpr : public ExprType<GroupingExpr> {
 public:
  std::unique_ptr<Expr> expr;

  explicit GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
};

class VarExpr : public ExprType<VarExpr> {
 public:
  std::string identifier;
  Position position;

  explicit VarExpr(std::string identifier, Position position)
      : identifier(std::move(identifier)), position(position){};
};

template <typename Derived>
class CastExpr : public ExprType<Derived> {
 public:
  std::unique_ptr<Expr> left;
  VarType type;
  Position position;

  CastExpr(std::unique_ptr<Expr> left, VarType type, Position position)
      : left(std::move(left)), type(std::move(type)), position(position){};
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
  std::string field_name;
  Position position;

  explicit FieldAccessExpr(std::unique_ptr<Expr> parent_struct,
                           std::string field_name, Position position)
      : parent_struct(std::move(parent_struct)),
        field_name(std::move(field_name)),
        position(position){};
};

#endif  // BOALANG_EXPR_HPP
