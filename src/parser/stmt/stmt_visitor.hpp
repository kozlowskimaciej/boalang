/*! @file stmt.hpp
    @brief Source-related classes.
*/

#ifndef BOALANG_STMT_VISITOR_HPP
#define BOALANG_STMT_VISITOR_HPP

// forward declarations
class Expression_Stmt;
class Print_Stmt;
class Var_Stmt;
class Assign_Stmt;
class Block_Stmt;
class If_Stmt;
class While_Stmt;

class Stmt_Visitor {
 public:
  virtual ~Stmt_Visitor() = default;

  virtual void visit_expression_stmt(const Expression_Stmt& stmt) = 0;
  virtual void visit_print_stmt(const Print_Stmt& stmt) = 0;
  virtual void visit_var_stmt(const Var_Stmt& stmt) = 0;
  // virtual void visit_assign_stmt(const Assign_Stmt& stmt) = 0;
  virtual void visit_block_stmt(const Block_Stmt& stmt) = 0;
  virtual void visit_if_stmt(const If_Stmt& stmt) = 0;
  virtual void visit_while_stmt(const While_Stmt& stmt) = 0;
};

#endif //BOALANG_STMT_VISITOR_HPP
