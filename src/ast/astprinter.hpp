/*! @file ast_printer.hpp
    @brief Ast printer.
*/

#ifndef BOALANG_ASTPRINTER_HPP
#define BOALANG_ASTPRINTER_HPP

#include <expr/expr.hpp>
#include <memory>
#include <optional>
#include <stmt/stmt.hpp>
#include <variant>
#include <vector>

class ASTPrinter : public ExprVisitor, public StmtVisitor {
 private:
  unsigned int indent_ = 0;

  void parenthesize(
      std::initializer_list<std::variant<const Expr*, const Stmt*>> exprstmts,
      std::optional<Token> token = std::nullopt);
  static void print_memory_info(const std::string& class_name,
                                const void* address);

 public:
  void print(Program* program);

  void visit(const Program& stmt) override;
  void visit(const PrintStmt& stmt) override;
  void visit(const IfStmt& stmt) override;
  void visit(const BlockStmt& stmt) override;
  void visit(const WhileStmt& stmt) override;
  void visit(const VarDeclStmt& stmt) override;
  void visit(const StructFieldStmt& stmt) override;
  void visit(const StructDeclStmt& stmt) override;
  void visit(const VariantDeclStmt& stmt) override;
  void visit(const AssignStmt& stmt) override;
  void visit(const CallStmt& stmt) override;
  void visit(const FuncParamStmt& stmt) override;
  void visit(const FuncStmt& stmt) override;
  void visit(const ReturnStmt& stmt) override;
  void visit(const LambdaFuncStmt& stmt) override;
  void visit(const InspectStmt& stmt) override;

  void visit(const BinaryExpr& expr) override;
  void visit(const GroupingExpr& expr) override;
  void visit(const LiteralExpr& expr) override;
  void visit(const UnaryExpr& expr) override;
  void visit(const VarExpr& expr) override;
  void visit(const LogicalExpr& expr) override;
  void visit(const CastExpr& expr) override;
  void visit(const InitalizerListExpr& expr) override;
  void visit(const CallExpr& expr) override;
  void visit(const FieldAccessExpr& expr) override;
};

#endif  // BOALANG_ASTPRINTER_HPP
