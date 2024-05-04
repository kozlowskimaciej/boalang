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

class IfStmt : public Stmt {
 public:
  const std::unique_ptr<Expr> condition;
  const std::unique_ptr<Stmt> then_branch;
  const std::unique_ptr<Stmt> else_branch;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch, std::unique_ptr<Stmt> else_branch)
  : condition(std::move(condition)), then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {};
  void accept(StmtVisitor& stmt_visitor) const override {
    stmt_visitor.visit_if_stmt(*this);
  };
};

class BlockStmt : public Stmt {
 public:
  const std::vector<std::unique_ptr<Stmt>> statements;

  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)) {};
  void accept(StmtVisitor& stmt_visitor) const override {
    stmt_visitor.visit_block_stmt(*this);
  };
};

class WhileStmt : public Stmt {
 public:
  const std::unique_ptr<Expr> condition;
  const std::unique_ptr<Stmt> body;

  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
  : condition(std::move(condition)), body(std::move(body)) {};
  void accept(StmtVisitor& stmt_visitor) const override {
    stmt_visitor.visit_while_stmt(*this);
  };
};

#endif  // BOALANG_STMT_HPP
