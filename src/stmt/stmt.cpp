#include "stmt.hpp"

void Program::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void PrintStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void IfStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void BlockStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void WhileStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void VarDeclStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void StructFieldStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void StructDeclStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void VariantDeclStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void AssignStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void CallStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void FuncParamStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void FuncStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void ReturnStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void LambdaFuncStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}

void InspectStmt::accept(StmtVisitor &stmt_visitor) const {
  stmt_visitor.visit(*this);
}
