/*! @file stmtvisitor.hpp
    @brief Statement visitor.
*/

#ifndef BOALANG_STMTVISITOR_HPP
#define BOALANG_STMTVISITOR_HPP

// forward declarations
class Program;
class PrintStmt;
class IfStmt;
class BlockStmt;
class WhileStmt;
class VarDeclStmt;
class AssignStmt;
class CallStmt;

class StmtVisitor {
 public:
  virtual ~StmtVisitor() = default;

  virtual void visit_program_stmt(const Program& stmt) = 0;
  virtual void visit_print_stmt(const PrintStmt& stmt) = 0;
  virtual void visit_if_stmt(const IfStmt& stmt) = 0;
  virtual void visit_block_stmt(const BlockStmt& stmt) = 0;
  virtual void visit_while_stmt(const WhileStmt& stmt) = 0;
  virtual void visit_vardecl_stmt(const VarDeclStmt& stmt) = 0;
  virtual void visit_assign_stmt(const AssignStmt& stmt) = 0;
  virtual void visit_call_stmt(const CallStmt& stmt) = 0;
};

#endif  // BOALANG_STMTVISITOR_HPP
