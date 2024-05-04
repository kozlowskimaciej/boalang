#include "astprinter.hpp"

#include <iostream>

void ASTPrinter::parenthesize(std::initializer_list<std::variant<const Expr*, const Stmt*>> exprstmts,
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

    std::visit([this](auto& member) {
      member->accept(*this);
    }, item);
  }
  --indent_;
}

void ASTPrinter::print(Program* program) {
  program->accept(*this);
}

void ASTPrinter::print_memory_info(const std::string& class_name,
                                   const void* address) {
  std::cout << class_name << " @ " << std::hex << std::showbase
          << reinterpret_cast<std::uintptr_t>(address) << " ";
}

void ASTPrinter::visit_program_stmt(const Program &stmt) {
  print_memory_info("Program", &stmt);
  for (const auto& item : stmt.statements) {
    parenthesize({item.get()});
  }
}

void ASTPrinter::visit_if_stmt(const IfStmt &stmt) {
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

void ASTPrinter::visit_block_stmt(const BlockStmt &stmt) {
  print_memory_info("BlockStmt", &stmt);
  for (const auto& item : stmt.statements) {
    parenthesize({item.get()});
  }
}
void ASTPrinter::visit_while_stmt(const WhileStmt &stmt) {
  print_memory_info("WhileStmt", &stmt);
  std::cout << "\nCondition:";
  parenthesize({stmt.condition.get()});
  std::cout << "\nBody:";
  parenthesize({stmt.body.get()});
}

void ASTPrinter::visit_print_stmt(const PrintStmt &stmt) {
  print_memory_info("PrintStmt", &stmt);
  parenthesize({stmt.expr.get()});
}

void ASTPrinter::visit_binary_expr(const BinaryExpr& expr) {
  print_memory_info("BinaryExpr", &expr);
  parenthesize({expr.left.get(), expr.right.get()}, expr.op_symbol);
}

void ASTPrinter::visit_grouping_expr(const GroupingExpr& expr) {
  print_memory_info("GroupingExpr", &expr);
  parenthesize({expr.expr.get()});
}

void ASTPrinter::visit_literal_expr(const LiteralExpr& expr) {
  print_memory_info("LiteralExpr", &expr);
  std::cout << expr.literal;
}

void ASTPrinter::visit_unary_expr(const UnaryExpr& expr) {
  print_memory_info("UnaryExpr", &expr);
  parenthesize({expr.right.get()}, expr.op_symbol);
}

void ASTPrinter::visit_var_expr(const VarExpr& expr) {
  print_memory_info("VarExpr", &expr);
  std::cout << "{" << expr.identifier << "}";
}

void ASTPrinter::visit_logical_expr(const LogicalExpr& expr) {
  print_memory_info("LogicalExpr", &expr);
  parenthesize({expr.left.get(), expr.right.get()}, expr.op_symbol);
}

void ASTPrinter::visit_cast_expr(const CastExpr& expr) {
  print_memory_info("CastExpr", &expr);
  parenthesize({expr.left.get(), expr.type.get()}, expr.op_symbol);
}

void ASTPrinter::visit_type_expr(const TypeExpr& expr) {
  print_memory_info("TypeExpr", &expr);
  std::cout << expr.type;
}

void ASTPrinter::visit_initalizerlist_expr(const InitalizerListExpr& expr) {
  print_memory_info("InitalizerListExpr", &expr);
  for (const auto& item : expr.list) {
    parenthesize({item.get()});
  }
}

void ASTPrinter::visit_call_expr(const CallExpr& expr) {
  print_memory_info("CallExpr", &expr);
  std::cout << "\nCallee:";
  parenthesize({expr.callee.get()});
  std::cout << "\nArguments:";
  for (const auto& arg : expr.arguments) {
    parenthesize({arg.get()});
  }
}

void ASTPrinter::visit_fieldaccess_expr(const FieldAccessExpr& expr) {
  if (expr.parent_struct) {
    expr.parent_struct->accept(*this);
  }
  std::cout << ' ';
  print_memory_info("FieldAccessExpr", &expr);
  std::cout << expr.field_name;
}
