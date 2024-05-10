#include "astprinter.hpp"

#include <iostream>

void ASTPrinter::parenthesize(
    std::initializer_list<std::variant<const Expr*, const Stmt*>> exprstmts,
    std::optional<Token> token) {
  if (token) {
    std::cout << token.value();
  }

  ++indent_;
  for (const auto& item : exprstmts) {
    std::cout << '\n';
    for (unsigned int i = 0; i < indent_; ++i) {
      if (i < indent_ - 1) {
        std::cout << '-';
      } else {
        std::cout << '>';
      }
    }

    std::visit([this](auto& member) { member->accept(*this); }, item);
  }
  --indent_;
}

void ASTPrinter::print(Program* program) {
  program->accept(*this);
  std::cout << '\n';
}

void ASTPrinter::print_memory_info(const std::string& class_name,
                                   const void* address) {
  std::cout << class_name << " @ " << std::hex << std::showbase
            << reinterpret_cast<std::uintptr_t>(address) << " ";
}

void ASTPrinter::visit(const Program& stmt) {
  print_memory_info("Program", &stmt);
  for (const auto& item : stmt.statements) {
    parenthesize({item.get()});
  }
}

void ASTPrinter::visit(const IfStmt& stmt) {
  print_memory_info("IfStmt", &stmt);
  std::cout << "\nCondition:";
  parenthesize({stmt.condition.get()});
  std::cout << "\nThen branch:";
  parenthesize({stmt.then_branch.get()});
  if (stmt.else_branch) {
    std::cout << "\nElse branch:";
    parenthesize({stmt.else_branch.get()});
  }
}

void ASTPrinter::visit(const BlockStmt& stmt) {
  print_memory_info("BlockStmt", &stmt);
  for (const auto& item : stmt.statements) {
    parenthesize({item.get()});
  }
}
void ASTPrinter::visit(const WhileStmt& stmt) {
  print_memory_info("WhileStmt", &stmt);
  std::cout << "\nCondition:";
  parenthesize({stmt.condition.get()});
  std::cout << "\nBody:";
  parenthesize({stmt.body.get()});
}

void ASTPrinter::visit(const PrintStmt& stmt) {
  print_memory_info("PrintStmt", &stmt);
  parenthesize({stmt.expr.get()});
}

void ASTPrinter::visit(const VarDeclStmt& stmt) {
  print_memory_info("VarDeclStmt", &stmt);
  std::cout << (stmt.mut ? "mut" : "") << " " << stmt.type << " {"
            << stmt.identifier << "}";
  parenthesize({stmt.initializer.get()});
}

void ASTPrinter::visit(const StructFieldStmt& stmt) {
  print_memory_info("StructFieldStmt", &stmt);
  std::cout << (stmt.mut ? "mut" : "") << " " << stmt.type << " {"
            << stmt.identifier << "}";
}
void ASTPrinter::visit(const StructDeclStmt& stmt) {
  print_memory_info("StructDeclStmt", &stmt);
  std::cout << "{" << stmt.identifier << "}";
  for (const auto& field : stmt.fields) {
    parenthesize({field.get()});
  }
}
void ASTPrinter::visit(const VariantDeclStmt& stmt) {
  print_memory_info("VariantDeclStmt", &stmt);
  std::cout << "{" << stmt.identifier << "} types:";
  for (const auto& param : stmt.params) {
    std::cout << " " << param;
  }
}

void ASTPrinter::visit(const AssignStmt& stmt) {
  print_memory_info("AssignStmt", &stmt);
  parenthesize({stmt.var.get(), stmt.value.get()});
}

void ASTPrinter::visit(const CallStmt& stmt) {
  print_memory_info("CallStmt", &stmt);
  std::cout << "\nCallee: {" << stmt.identifier << "}";
  std::cout << "\nArguments:";
  for (const auto& arg : stmt.arguments) {
    parenthesize({arg.get()});
  }
}

void ASTPrinter::visit(const FuncParamStmt& stmt) {
  print_memory_info("FuncParamStmt", &stmt);
  std::cout << stmt.type << " {" << stmt.identifier << "}";
}

void ASTPrinter::visit(const FuncStmt& stmt) {
  print_memory_info("FuncStmt", &stmt);
  std::cout << "[return type: " << stmt.return_type << "] {" << stmt.identifier
            << "}";
  std::cout << "\nParams:";
  for (const auto& param : stmt.params) {
    parenthesize({param.get()});
  }
  std::cout << "\nBody:";
  parenthesize({stmt.body.get()});
}

void ASTPrinter::visit(const ReturnStmt& stmt) {
  print_memory_info("ReturnStmt", &stmt);
  parenthesize({stmt.value.get()});
}

void ASTPrinter::visit(const LambdaFuncStmt& stmt) {
  print_memory_info("LambdaFuncStmt", &stmt);
  std::cout << "[type: " << stmt.type << "] {" << stmt.identifier << "}";
  std::cout << "\nBody:";
  parenthesize({stmt.body.get()});
}

void ASTPrinter::visit(const InspectStmt& stmt) {
  print_memory_info("InspectStmt", &stmt);
  std::cout << "\nInspected:";
  parenthesize({stmt.inspected.get()});
  std::cout << "\nLambdas:";
  for (const auto& lambda : stmt.lambdas) {
    parenthesize({lambda.get()});
  }
  if (stmt.default_lambda) {
    std::cout << "\nDefault:";
    parenthesize({stmt.default_lambda.get()});
  }
}

void ASTPrinter::visit(const BinaryExpr& expr) {
  print_memory_info("BinaryExpr", &expr);
  parenthesize({expr.left.get(), expr.right.get()}, expr.op_symbol);
}

void ASTPrinter::visit(const GroupingExpr& expr) {
  print_memory_info("GroupingExpr", &expr);
  parenthesize({expr.expr.get()});
}

void ASTPrinter::visit(const LiteralExpr& expr) {
  print_memory_info("LiteralExpr", &expr);
  std::cout << expr.literal;
}

void ASTPrinter::visit(const UnaryExpr& expr) {
  print_memory_info("UnaryExpr", &expr);
  parenthesize({expr.right.get()}, expr.op_symbol);
}

void ASTPrinter::visit(const VarExpr& expr) {
  print_memory_info("VarExpr", &expr);
  std::cout << "{" << expr.identifier << "}";
}

void ASTPrinter::visit(const LogicalExpr& expr) {
  print_memory_info("LogicalExpr", &expr);
  parenthesize({expr.left.get(), expr.right.get()}, expr.op_symbol);
}

void ASTPrinter::visit(const CastExpr& expr) {
  print_memory_info("CastExpr", &expr);
  std::cout << "[target type: " << expr.type << "] ";
  parenthesize({expr.left.get()}, expr.op_symbol);
}

void ASTPrinter::visit(const InitalizerListExpr& expr) {
  print_memory_info("InitalizerListExpr", &expr);
  for (const auto& item : expr.list) {
    parenthesize({item.get()});
  }
}

void ASTPrinter::visit(const CallExpr& expr) {
  print_memory_info("CallExpr", &expr);
  std::cout << "\nCallee:";
  parenthesize({expr.callee.get()});
  std::cout << "\nArguments:";
  for (const auto& arg : expr.arguments) {
    parenthesize({arg.get()});
  }
}

void ASTPrinter::visit(const FieldAccessExpr& expr) {
  if (expr.parent_struct) {
    expr.parent_struct->accept(*this);
  }
  std::cout << ' ';
  print_memory_info("FieldAccessExpr", &expr);
  std::cout << expr.field_name;
}
