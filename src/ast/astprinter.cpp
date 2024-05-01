#include <iostream>
#include <iomanip> // for std::hex

#include "astprinter.hpp"

void ASTPrinter::parenthesize(std::initializer_list<const Expr*> exprs, std::optional<Token> token) {
  if (token) {
    stream_ << token.value();
  }

  stream_ << '(';
  for (const auto& expr : exprs) {
    expr->accept(*this);
    stream_ << ", ";
  }
  stream_ << ')';
}

void ASTPrinter::print(const std::vector<std::unique_ptr<Expr>>& exprs) {
  for (const auto& expr : exprs) {
    expr->accept(*this);
    stream_ << '\n';
  }
  std::cout << stream_.str();
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
  stream_ << "{" << expr.literal.stringify() << "}";
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
      {expr.right.get(), expr.left.get()},
      expr.op_symbol
  );
}

void ASTPrinter::visit_assign_expr(const AssignExpr& expr) {
  print_memory_info("AssignExpr", &expr);
  parenthesize(
      {expr.value.get()},
      expr.name
  );
}

void ASTPrinter::print_memory_info(const std::string& class_name, const void* address) {
  stream_ << class_name << " @ " << std::hex << std::showbase << reinterpret_cast<std::uintptr_t>(address) << " ";
}
