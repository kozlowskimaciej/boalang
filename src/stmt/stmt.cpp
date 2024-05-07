#include "stmt.hpp"

void Program::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_program_stmt(*this);
}

void PrintStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_print_stmt(*this);
}

void IfStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_if_stmt(*this);
}

void BlockStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_block_stmt(*this);
}

void WhileStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_while_stmt(*this);
}

void VarDeclStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_vardecl_stmt(*this);
}

void StructFieldStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_structfield_stmt(*this);
}

void StructDeclStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_structdecl_stmt(*this);
}

void VariantDeclStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_variantdecl_stmt(*this);
}

void AssignStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_assign_stmt(*this);
}

void CallStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_call_stmt(*this);
}

void FuncParamStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_funcparam_stmt(*this);
}

void FuncStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_func_stmt(*this);
}

void ReturnStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_return_stmt(*this);
}

void LambdaFuncStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_lambdafunc_stmt(*this);
}

void InspectStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit_inspect_stmt(*this);
}
