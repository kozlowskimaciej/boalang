/*! @file stmt.hpp
    @brief Statements.
*/

#ifndef BOALANG_STMT_HPP
#define BOALANG_STMT_HPP

#include <memory>
#include <vector>

#include "expr/expr.hpp"

class Program;
class PrintStmt;
class IfStmt;
class BlockStmt;
class WhileStmt;
class VarDeclStmt;
class StructFieldStmt;
class StructDeclStmt;
class VariantDeclStmt;
class AssignStmt;
class CallStmt;
class FuncParamStmt;
class FuncStmt;
class ReturnStmt;
class LambdaFuncStmt;
class InspectStmt;

/**
 * @brief Interface for statements visitor.
 */
class StmtVisitor {
 public:
  virtual ~StmtVisitor() = default;

  StmtVisitor() = default;
  StmtVisitor(const StmtVisitor&) = delete;
  StmtVisitor& operator=(const StmtVisitor&) = delete;

  StmtVisitor(StmtVisitor&&) = default;
  StmtVisitor& operator=(StmtVisitor&&) = default;

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

/**
 * @brief Interface for statements.
 */
class Stmt {
 public:
  Position position;

  virtual ~Stmt() = default;
  virtual void accept(StmtVisitor& stmt_visitor) const = 0;

  Stmt(Position position) : position(position){};
  Stmt(const Stmt&) = delete;
  Stmt& operator=(const Stmt&) = delete;

  Stmt(Stmt&&) = default;
  Stmt& operator=(Stmt&&) = default;
};

template <typename Derived>
class StmtType : public Stmt {
 public:
  using Stmt::Stmt;
  void accept(StmtVisitor& visitor) const override {
    visitor.visit(static_cast<const Derived&>(*this));
  }
};

class Program : public StmtType<Program> {
 public:
  std::vector<std::unique_ptr<Stmt>> statements;

  explicit Program(std::vector<std::unique_ptr<Stmt>> statements,
                   Position position)
      : StmtType(position), statements(std::move(statements)){};
};

class PrintStmt : public StmtType<PrintStmt> {
 public:
  std::unique_ptr<Expr> expr;

  explicit PrintStmt(std::unique_ptr<Expr> expr, Position position)
      : StmtType(position), expr(std::move(expr)){};
};

class IfStmt : public StmtType<IfStmt> {
 public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> then_branch;
  std::unique_ptr<Stmt> else_branch;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch,
         std::unique_ptr<Stmt> else_branch, Position position)
      : StmtType(position),
        condition(std::move(condition)),
        then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)){};
};

class BlockStmt : public StmtType<BlockStmt> {
 public:
  std::vector<std::unique_ptr<Stmt>> statements;

  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements,
                     Position position)
      : StmtType(position), statements(std::move(statements)){};
};

class WhileStmt : public StmtType<WhileStmt> {
 public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;

  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body,
            Position position)
      : StmtType(position),
        condition(std::move(condition)),
        body(std::move(body)){};
};

class VarDeclStmt : public StmtType<VarDeclStmt> {
 public:
  VarType type;
  std::string identifier;
  std::unique_ptr<Expr> initializer;
  bool mut;

  VarDeclStmt(VarType type, std::string identifier,
              std::unique_ptr<Expr> initializer, Position position,
              bool mut = false)
      : StmtType(position),
        type(std::move(type)),
        identifier(std::move(identifier)),
        initializer(std::move(initializer)),
        mut(mut){};
};

class StructFieldStmt : public StmtType<StructFieldStmt> {
 public:
  VarType type;
  std::string identifier;
  bool mut;

  StructFieldStmt(VarType type, std::string identifier, Position position,
                  bool mut = false)
      : StmtType(position),
        type(std::move(type)),
        identifier(std::move(identifier)),
        mut(mut){};
};

class StructDeclStmt : public StmtType<StructDeclStmt> {
 public:
  std::string identifier;
  std::vector<std::unique_ptr<StructFieldStmt>> fields;

  StructDeclStmt(std::string identifier,
                 std::vector<std::unique_ptr<StructFieldStmt>> fields,
                 Position position)
      : StmtType(position),
        identifier(std::move(identifier)),
        fields(std::move(fields)){};
};

class VariantDeclStmt : public StmtType<VariantDeclStmt> {
 public:
  std::string identifier;
  std::vector<VarType> params;

  VariantDeclStmt(std::string identifier, std::vector<VarType> params,
                  Position position)
      : StmtType(position),
        identifier(std::move(identifier)),
        params(std::move(params)){};
};

class AssignStmt : public StmtType<AssignStmt> {
 public:
  std::unique_ptr<Expr> var;
  std::unique_ptr<Expr> value;

  AssignStmt(std::unique_ptr<Expr> var, std::unique_ptr<Expr> value,
             Position position)
      : StmtType(position), var(std::move(var)), value(std::move(value)){};
};

class CallStmt : public StmtType<CallStmt> {
 public:
  std::string identifier;
  std::vector<std::unique_ptr<Expr>> arguments;

  explicit CallStmt(std::string identifier, Position position,
                    std::vector<std::unique_ptr<Expr>> arguments = {})
      : StmtType(position),
        identifier(std::move(identifier)),
        arguments(std::move(arguments)){};
};

class FuncParamStmt : public StmtType<FuncParamStmt> {
 public:
  VarType type;
  std::string identifier;

  FuncParamStmt(VarType type, std::string identifier, Position position)
      : StmtType(position),
        type(std::move(type)),
        identifier(std::move(identifier)){};
};

class FuncStmt : public StmtType<FuncStmt> {
 public:
  std::string identifier;
  VarType return_type;
  std::vector<std::unique_ptr<FuncParamStmt>> params;
  std::unique_ptr<Stmt> body;

  FuncStmt(std::string identifier, VarType return_type,
           std::vector<std::unique_ptr<FuncParamStmt>> params,
           std::unique_ptr<Stmt> body, Position position)
      : StmtType(position),
        identifier(std::move(identifier)),
        return_type(std::move(return_type)),
        params(std::move(params)),
        body(std::move(body)){};
};

class ReturnStmt : public StmtType<ReturnStmt> {
 public:
  std::unique_ptr<Expr> value;

  ReturnStmt(std::unique_ptr<Expr> value, Position position)
      : StmtType(position), value(std::move(value)){};
};

class LambdaFuncStmt : public StmtType<LambdaFuncStmt> {
 public:
  VarType type;
  std::string identifier;
  std::unique_ptr<Stmt> body;

  LambdaFuncStmt(VarType type, std::string identifier,
                 std::unique_ptr<Stmt> body, Position position)
      : StmtType(position),
        type(std::move(type)),
        identifier(std::move(identifier)),
        body(std::move(body)){};
};

class InspectStmt : public StmtType<InspectStmt> {
 public:
  std::unique_ptr<Expr> inspected;
  std::vector<std::unique_ptr<LambdaFuncStmt>> lambdas;
  std::unique_ptr<Stmt> default_lambda;

  InspectStmt(std::unique_ptr<Expr> inspected,
              std::vector<std::unique_ptr<LambdaFuncStmt>> lambdas,
              Position position, std::unique_ptr<Stmt> default_lambda = nullptr)
      : StmtType(position),
        inspected(std::move(inspected)),
        lambdas(std::move(lambdas)),
        default_lambda(std::move(default_lambda)){};
};

#endif  // BOALANG_STMT_HPP
