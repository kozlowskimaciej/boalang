#include "interpreter.hpp"

#include <iostream>
#include <cassert>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

eval_value_t Interpreter::evaluate(const Expr* expr) {
  expr->accept(*this);
  return get_evaluation();
}

void Interpreter::set_evaluation(eval_value_t value) {
  assert(!evaluation.has_value());
  evaluation = std::move(value);
}

eval_value_t Interpreter::get_evaluation() {
  assert(evaluation);
  auto value = evaluation;
  evaluation.reset();
  return *value;
}

void Interpreter::visit(const Program& stmt) {
  for (const auto& s : stmt.statements) {
    s->accept(*this);
  }
}

void Interpreter::visit(const PrintStmt& stmt) {
  auto value = evaluate(stmt.expr.get());

  std::visit(overloaded{
      [](const value_t& v) {
        return std::visit(overloaded{
            [](auto) { throw RuntimeError("Value unprintable"); },
            [](int arg) { std::cout << std::to_string(arg); },
            [](float arg) { std::cout << std::to_string(arg); },
            [](const std::string& arg) { std::cout << arg; },
            [](bool arg) { std::cout << std::string(arg ? "true" : "false"); },
        }, v);
      },
      [](auto) { throw RuntimeError("Value unprintable"); },  // &stmt
  }, value);

  std::cout << '\n';
}

void Interpreter::visit(const LiteralExpr& expr) {
  set_evaluation(expr.literal);
}
void Interpreter::visit(const IfStmt &stmt) {

}
void Interpreter::visit(const BlockStmt &stmt) {

}
void Interpreter::visit(const WhileStmt &stmt) {

}
void Interpreter::visit(const VarDeclStmt &stmt) {

}
void Interpreter::visit(const StructFieldStmt &stmt) {

}
void Interpreter::visit(const StructDeclStmt &stmt) {

}
void Interpreter::visit(const VariantDeclStmt &stmt) {

}
void Interpreter::visit(const AssignStmt &stmt) {

}
void Interpreter::visit(const CallStmt &stmt) {

}
void Interpreter::visit(const FuncParamStmt &stmt) {

}
void Interpreter::visit(const FuncStmt &stmt) {

}
void Interpreter::visit(const ReturnStmt &stmt) {

}
void Interpreter::visit(const LambdaFuncStmt &stmt) {

}
void Interpreter::visit(const InspectStmt &stmt) {

}
void Interpreter::visit(const AdditionExpr &expr) {

}
void Interpreter::visit(const SubtractionExpr &expr) {

}
void Interpreter::visit(const DivisionExpr &expr) {

}
void Interpreter::visit(const MultiplicationExpr &expr) {

}
void Interpreter::visit(const EqualCompExpr &expr) {

}
void Interpreter::visit(const NotEqualCompExpr &expr) {

}
void Interpreter::visit(const GreaterCompExpr &expr) {

}
void Interpreter::visit(const GreaterEqualCompExpr &expr) {

}
void Interpreter::visit(const LessCompExpr &expr) {

}
void Interpreter::visit(const LessEqualCompExpr &expr) {

}
void Interpreter::visit(const GroupingExpr &expr) {

}
void Interpreter::visit(const NegationExpr &expr) {
  auto value = evaluate(expr.right.get());

  std::visit(overloaded{
      [this](const value_t& v) {
        return std::visit(overloaded{
            [](auto) { throw RuntimeError("Value unnegatable"); },
            [this](int arg) { set_evaluation(-arg); },
            [this](float arg) { set_evaluation(-arg); },
        }, v);
      },
      [](auto) { throw RuntimeError("Value unnegatable"); },
  }, value);
//  std::visit(overloaded{
//      [](const value_t& v) {
//        return std::visit(overloaded{
//            [](std::monostate) { throw RuntimeError("Not printable"); },
//            [](int arg) { std::cout << std::to_string(arg); },
//            [](float arg) { std::cout << std::to_string(arg); },
//            [](const std::string& arg) { std::cout << arg; },
//            [](bool arg) { std::cout << std::string(arg ? "true" : "false"); },
//        }, v);
//      },
//      [](const std::shared_ptr<Variable>&) { throw RuntimeError("Not printable"); },  // &stmt
//      [](const std::shared_ptr<StructObject>&) { throw RuntimeError("Not printable"); },
//      [](const std::shared_ptr<VariantObject>&) { throw RuntimeError("Not printable"); },
//  }, value);
}
void Interpreter::visit(const LogicalNegationExpr &expr) {

}
void Interpreter::visit(const VarExpr &expr) {

}
void Interpreter::visit(const LogicalOrExpr &expr) {

}
void Interpreter::visit(const LogicalAndExpr &expr) {

}
void Interpreter::visit(const IsTypeExpr &expr) {

}
void Interpreter::visit(const AsTypeExpr &expr) {

}
void Interpreter::visit(const InitalizerListExpr &expr) {

}
void Interpreter::visit(const CallExpr &expr) {

}
void Interpreter::visit(const FieldAccessExpr &expr) {

}

#pragma clang diagnostic pop
