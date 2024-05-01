/*! @file expr.hpp
    @brief Expressions.
*/

#ifndef BOALANG_EXPR_HPP
#define BOALANG_EXPR_HPP

#include <memory>

#include "token/token.hpp"


class Expr {
 public:
  virtual ~Expr() = default;

//  virtual void accept(ExprVisitor& expr_visitor) const = 0;
};

class Assign_Expr : public Expr {
 public:
  const Token name;
  const std::unique_ptr<Expr> value;

  Assign_Expr(Token name, Expr* value)
      : name(name), value(value){};
};

class Binary_Expr : public Expr {
 public:
  const std::unique_ptr<Expr> left;
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  Binary_Expr(Expr* left, Token op_symbol, Expr* right)
      : left(left), op_symbol(op_symbol), right(right){};
};

class Unary_Expr : public Expr {
 public:
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  Unary_Expr(Token op_symbol, Expr* right)
      : op_symbol(op_symbol), right(right){};
};

class Logical_Expr : public Expr {
 public:
  const std::unique_ptr<Expr> left;
  const Token op_symbol;
  const std::unique_ptr<Expr> right;

  Logical_Expr(Expr* left, Token op_symbol, Expr* right)
      : left(left), op_symbol(op_symbol), right(right){};
};

class Literal_Expr : public Expr {
 public:
  const Token literal;

  Literal_Expr(Token literal) : literal(literal){};
};

#endif //BOALANG_EXPR_HPP
