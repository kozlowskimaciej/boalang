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

bool Interpreter::boolify(const eval_value_t &value) {
  return std::visit(overloaded{
      [](const value_t& v) {
        return std::visit(overloaded{
            [](auto) { return false; },
            [](int arg) { return arg != 0; },
            [](float arg) { return arg != 0.F; },
            [](const std::string& arg) { return !arg.empty(); },
            [](bool arg) { return arg; },
        }, v);
      },
      [](auto) { return true; },
  }, value);
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
  if (boolify(evaluate(stmt.condition.get()))) {
    stmt.then_branch->accept(*this);
  } else if (stmt.else_branch) {
    stmt.else_branch->accept(*this);
  }
}

void Interpreter::visit(const BlockStmt &stmt) {
  scopes.emplace_back();
  std::exception_ptr eptr = nullptr;
  try {
    for (const auto& s : stmt.statements) {
      s->accept(*this);
    }
  } catch(...) {
    eptr = std::current_exception();
  }
  scopes.pop_back();
  if (eptr) { std::rethrow_exception(eptr); }
}

void Interpreter::visit(const WhileStmt &stmt) {
  while (boolify(evaluate(stmt.condition.get()))) {
    stmt.body->accept(*this);
  }
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
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [this, &expr](const value_t& lhs, const value_t& rhs) {
        return std::visit(overloaded{
          [this](int lhs, int rhs) { set_evaluation(lhs + rhs); },
          [this](float lhs, float rhs) { set_evaluation(lhs + rhs); },
          [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs + rhs); },
          [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot add"); }
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot add"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const SubtractionExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [this, &expr](const value_t& lhs, const value_t& rhs) {
        return std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs - rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs - rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot substract"); }
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot substract"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const DivisionExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [this, &expr](const value_t& lhs, const value_t& rhs) {
        return std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs / rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs / rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot divide"); }
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot divide"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const MultiplicationExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [this, &expr](const value_t& lhs, const value_t& rhs) {
        return std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs * rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs * rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot multiply"); }
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot multiply"); },
  }, leftValue, rightValue);
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
  set_evaluation(evaluate(expr.expr.get()));
}

void Interpreter::visit(const NegationExpr &expr) {
  auto value = evaluate(expr.right.get());
  set_evaluation(boolify(value));
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
  auto value = evaluate(expr.right.get());
  set_evaluation(!boolify(value));
}

void Interpreter::visit(const VarExpr &expr) {

}

void Interpreter::visit(const LogicalOrExpr &expr) {
  auto right = evaluate(expr.right.get());
  auto left = evaluate(expr.left.get());
  set_evaluation(boolify(right) || boolify(left));
}

void Interpreter::visit(const LogicalAndExpr &expr) {
  auto right = evaluate(expr.right.get());
  auto left = evaluate(expr.left.get());
  set_evaluation(boolify(right) && boolify(left));
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
