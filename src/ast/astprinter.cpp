#include <iostream>
#include <iomanip> // for std::hex

#include "astprinter.hpp"

void ASTPrinter::parenthesize(std::initializer_list<const Expr*> exprs, std::optional<Token> token) {
  if (token) {
    stream_ << token.value();
  }

  ++indent_;
  for (const auto& expr : exprs) {
    stream_ << '\n';
    for (unsigned int i = 0; i < indent_; ++i) {
      if (i < indent_ - 1) {
        stream_ << '-';
      } else {
        stream_ << '>';
      }
    }
    expr->accept(*this);
  }
  --indent_;
}

void ASTPrinter::print(const std::vector<std::unique_ptr<Expr>>& exprs) {
  for (const auto& expr : exprs) {
    expr->accept(*this);
    stream_ << '\n';
  }
  std::cout << stream_.str();
}

void ASTPrinter::print_memory_info(const std::string& class_name, const void* address) {
  stream_ << class_name << " @ " << std::hex << std::showbase << reinterpret_cast<std::uintptr_t>(address) << " ";
}

void ASTPrinter::visit_binary_expr(const BinaryExpr& expr) {
  print_memory_info("BinaryExpr", &expr);
  parenthesize(
      {expr.left.get(), expr.right.get()},
      expr.op_symbol
  );
}

void ASTPrinter::visit_grouping_expr(const GroupingExpr& expr) {
  print_memory_info("GroupingExpr", &expr);
  parenthesize(
      {expr.expr.get()}
  );
}

void ASTPrinter::visit_literal_expr(const LiteralExpr& expr) {
  print_memory_info("LiteralExpr", &expr);
  stream_ << expr.literal;
}

void ASTPrinter::visit_unary_expr(const UnaryExpr& expr) {
  print_memory_info("UnaryExpr", &expr);
  parenthesize(
      {expr.right.get()},
      expr.op_symbol
  );
}

void ASTPrinter::visit_var_expr(const VarExpr& expr) {
  print_memory_info("VarExpr", &expr);
  stream_ << "{" << expr.identifier << "}";
}

void ASTPrinter::visit_logical_expr(const LogicalExpr& expr) {
  print_memory_info("LogicalExpr", &expr);
  parenthesize(
      {expr.left.get(), expr.right.get()},
      expr.op_symbol
  );
}

void ASTPrinter::visit_cast_expr(const CastExpr &expr) {
  print_memory_info("CastExpr", &expr);
  parenthesize(
      {expr.left.get(), expr.type.get()},
      expr.op_symbol
  );
}

void ASTPrinter::visit_type_expr(const TypeExpr &expr) {
  print_memory_info("TypeExpr", &expr);
  stream_ << expr.type;
}

void ASTPrinter::visit_initalizerlist_expr(const InitalizerListExpr &expr) {
  print_memory_info("InitalizerListExpr", &expr);
  for (const auto& item : expr.list) {
    parenthesize({item.get()});
  }
}

void ASTPrinter::visit_call_expr(const CallExpr &expr) {
  print_memory_info("CallExpr", &expr);
  stream_ << "\nCallee:";
  parenthesize({expr.callee.get()});
  stream_ << "\nArguments:";
  for (const auto& arg : expr.arguments) {
    parenthesize({arg.get()});
  }
}

void ASTPrinter::visit_fieldaccess_expr(const FieldAccessExpr &expr) {
  if (expr.parent_struct) {
    expr.parent_struct->accept(*this);
  }
  stream_ << ' ';
  print_memory_info("FieldAccessExpr", &expr);
  stream_ << expr.field_name;
}
