/*! @file stmt.hpp
    @brief Statements.
*/

#ifndef BOALANG_STMT_HPP
#define BOALANG_STMT_HPP

#include "stmtvisitor.hpp"

class Stmt {
 public:
  virtual ~Stmt() = default;
  virtual void accept(StmtVisitor& stmt_visitor) const = 0;
};

class Program : public Stmt {
 public:
  const std::vector<std::unique_ptr<Stmt>> statements;

  explicit Program(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)) {};
  void accept(StmtVisitor& stmt_visitor) const override {
    stmt_visitor.visit_program_stmt(*this);
  };
};

class PrintStmt : public Stmt {
 public:
  const std::unique_ptr<Expr> expr;

  explicit PrintStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {};
  void accept(StmtVisitor& stmt_visitor) const override {
    stmt_visitor.visit_print_stmt(*this);
  };
};

#endif  // BOALANG_STMT_HPP
