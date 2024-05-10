/*! @file stmt.hpp
    @brief Statements.
*/

#ifndef BOALANG_STMT_HPP
#define BOALANG_STMT_HPP

#include <memory>
#include <vector>

#include "expr/expr.hpp"

class StmtVisitor;

class Stmt {
 public:
  virtual ~Stmt() = default;
  virtual void accept(StmtVisitor& stmt_visitor) const = 0;
};

class Program : public Stmt {
 public:
  std::vector<std::unique_ptr<Stmt>> statements;

  explicit Program(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class PrintStmt : public Stmt {
 public:
  std::unique_ptr<Expr> expr;

  explicit PrintStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class IfStmt : public Stmt {
 public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> then_branch;
  std::unique_ptr<Stmt> else_branch;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch,
         std::unique_ptr<Stmt> else_branch)
      : condition(std::move(condition)),
        then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class BlockStmt : public Stmt {
 public:
  std::vector<std::unique_ptr<Stmt>> statements;

  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class WhileStmt : public Stmt {
 public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;

  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
      : condition(std::move(condition)), body(std::move(body)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class VarDeclStmt : public Stmt {
 public:
  Token type;
  Token identifier;
  std::unique_ptr<Expr> initializer;
  bool mut;

  VarDeclStmt(Token type, Token identifier, std::unique_ptr<Expr> initializer,
              bool mut = false)
      : type(std::move(type)),
        identifier(std::move(identifier)),
        initializer(std::move(initializer)),
        mut(mut){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class StructFieldStmt : public Stmt {
 public:
  Token type;
  Token identifier;
  bool mut;

  StructFieldStmt(Token type, Token identifier, bool mut = false)
      : type(std::move(type)), identifier(std::move(identifier)), mut(mut){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class StructDeclStmt : public Stmt {
 public:
  Token identifier;
  std::vector<std::unique_ptr<StructFieldStmt>> fields;

  StructDeclStmt(Token identifier,
                 std::vector<std::unique_ptr<StructFieldStmt>> fields)
      : identifier(std::move(identifier)), fields(std::move(fields)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class VariantDeclStmt : public Stmt {
 public:
  Token identifier;
  std::vector<Token> params;

  VariantDeclStmt(Token identifier, std::vector<Token> params)
      : identifier(std::move(identifier)), params(std::move(params)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class AssignStmt : public Stmt {
 public:
  std::unique_ptr<Expr> var;
  std::unique_ptr<Expr> value;

  AssignStmt(std::unique_ptr<Expr> var, std::unique_ptr<Expr> value)
      : var(std::move(var)), value(std::move(value)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class CallStmt : public Stmt {
 public:
  Token identifier;
  std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallStmt(Token identifier,
                    std::vector<std::unique_ptr<Expr>> arguments = {})
      : identifier(std::move(identifier)), arguments(std::move(arguments)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class FuncParamStmt : public Stmt {
 public:
  Token type;
  Token identifier;

  FuncParamStmt(Token type, Token identifier)
      : type(std::move(type)), identifier(std::move(identifier)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class FuncStmt : public Stmt {
 public:
  Token identifier;
  Token return_type;
  std::vector<std::unique_ptr<FuncParamStmt>> params;
  std::unique_ptr<BlockStmt> body;

  FuncStmt(Token identifier, Token return_type,
           std::vector<std::unique_ptr<FuncParamStmt>> params,
           std::unique_ptr<BlockStmt> body)
      : identifier(std::move(identifier)),
        return_type(std::move(return_type)),
        params(std::move(params)),
        body(std::move(body)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class ReturnStmt : public Stmt {
 public:
  std::unique_ptr<Expr> value;

  ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class LambdaFuncStmt : public Stmt {
 public:
  Token type;
  Token identifier;
  std::unique_ptr<Stmt> body;

  LambdaFuncStmt(Token type, Token identifier, std::unique_ptr<Stmt> body)
      : type(std::move(type)),
        identifier(std::move(identifier)),
        body(std::move(body)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class InspectStmt : public Stmt {
 public:
  std::unique_ptr<Expr> inspected;
  std::vector<std::unique_ptr<LambdaFuncStmt>> lambdas;
  std::unique_ptr<Stmt> default_lambda;

  InspectStmt(std::unique_ptr<Expr> inspected,
              std::vector<std::unique_ptr<LambdaFuncStmt>> lambdas,
              std::unique_ptr<Stmt> default_lambda = nullptr)
      : inspected(std::move(inspected)),
        lambdas(std::move(lambdas)),
        default_lambda(std::move(default_lambda)){};
  void accept(StmtVisitor& stmt_visitor) const override;
};

class StmtVisitor {
 public:
  virtual ~StmtVisitor() = default;

  virtual void visit(const Program& stmt) = 0;
  virtual void visit(const PrintStmt& stmt) = 0;
  virtual void visit(const IfStmt& stmt) = 0;
  virtual void visit(const BlockStmt& stmt) = 0;
  virtual void visit(const WhileStmt& stmt) = 0;
  virtual void visit(const VarDeclStmt& stmt) = 0;
  virtual void visit(const StructFieldStmt& stmt) = 0;
  virtual void visit(const StructDeclStmt& stmt) = 0;
  virtual void visit(const VariantDeclStmt& stmt) = 0;
  virtual void visit(const AssignStmt& stmt) = 0;
  virtual void visit(const CallStmt& stmt) = 0;
  virtual void visit(const FuncParamStmt& stmt) = 0;
  virtual void visit(const FuncStmt& stmt) = 0;
  virtual void visit(const ReturnStmt& stmt) = 0;
  virtual void visit(const LambdaFuncStmt& stmt) = 0;
  virtual void visit(const InspectStmt& stmt) = 0;
};

#endif  // BOALANG_STMT_HPP
