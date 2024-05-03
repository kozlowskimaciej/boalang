/*! @file stmtvisitor.hpp
    @brief Statement visitor.
*/

#ifndef BOALANG_STMTVISITOR_HPP
#define BOALANG_STMTVISITOR_HPP

// forward declarations
class Program;
class PrintStmt;

class StmtVisitor {
 public:
  virtual ~StmtVisitor() = default;

  virtual void visit_program_stmt(const Program& stmt) = 0;
  virtual void visit_print_stmt(const PrintStmt& stmt) = 0;
};

#endif  // BOALANG_STMTVISITOR_HPP
