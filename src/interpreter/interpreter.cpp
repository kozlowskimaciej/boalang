#include "interpreter.hpp"

#include <iostream>
#include <cassert>
#include <cmath>

#include "utils/position.hpp"

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
  assert(evaluation.has_value());
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
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs == rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs == rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs == rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs == rhs); },
            [this](auto, auto) { set_evaluation(false); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const NotEqualCompExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs != rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs != rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs != rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs != rhs); },
            [this](auto, auto) { set_evaluation(true); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const GreaterCompExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs > rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs > rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs > rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs > rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const GreaterEqualCompExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs >= rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs >= rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs >= rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs >= rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const LessCompExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs < rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs < rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs < rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs < rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const LessEqualCompExpr &expr) {
  auto leftValue = evaluate(expr.left.get());
  auto rightValue = evaluate(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs <= rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs <= rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs <= rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs <= rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const GroupingExpr &expr) {
  set_evaluation(evaluate(expr.expr.get()));
}

void Interpreter::visit(const NegationExpr &expr) {
  auto value = evaluate(expr.right.get());
  set_evaluation(boolify(value));
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
  auto left = evaluate(expr.left.get());
  auto type = expr.type;
  bool value = std::visit(overloaded{
    [&type](const value_t& v) {
      return std::visit(overloaded{
          [](auto) { return false; },
          [&type](int arg) { return type.type == INT; },
          [&type](float arg) { return type.type == FLOAT; },
          [&type](const std::string& arg) { return type.type == STR; },
          [&type](bool arg) { return type.type == BOOL; },
      }, v);
    },
    [&type](const std::shared_ptr<Variable>& arg) { return arg->type.name == type.name; },
    [&type](const std::shared_ptr<StructObject>& arg) { return arg->type_name == type.name; },
    [&type](const std::shared_ptr<VariantObject>& arg) { return arg->type_name == type.name; },
    [](auto) { throw RuntimeError("Invalid type comparison"); },
  }, left);
  set_evaluation(value);
}

void Interpreter::visit(const AsTypeExpr &expr) {
  auto left = evaluate(expr.left.get());
  auto type = expr.type;

  if (type.type == BOOL) {
    set_evaluation(boolify(left));
    return;
  }

  std::visit(overloaded{
      [this, &type](const value_t& v) {
        std::visit(overloaded{
            [](auto) { throw RuntimeError("Invalid type cast"); },
            [this, &type](int arg) {
              switch (type.type) {
                case INT:
                  set_evaluation(arg);
                  break;
                case FLOAT:
                  set_evaluation(static_cast<float>(arg));
                  break;
                case STR:
                  set_evaluation(std::to_string(arg));
                  break;
                default:
                  set_evaluation(arg);
                  break;
              }
            },
            [this, &type](float arg) {
              switch (type.type) {
                case INT:
                  set_evaluation(static_cast<int>(std::round(arg)));
                  break;
                case FLOAT:
                  set_evaluation(arg);
                  break;
                case STR:
                  set_evaluation(std::to_string(arg));
                  break;
                default:
                  throw RuntimeError("Invalid type cast");
              }
            },
            [this, &type](const std::string& arg) {
              switch (type.type) {
                case STR:
                  set_evaluation(arg);
                  break;
                default:
                  throw RuntimeError("Invalid type cast");
              }
            },
            [this, &type](bool arg) {
              switch (type.type) {
                case STR:
                  set_evaluation(arg?"true":"false");
                  break;
                default:
                  throw RuntimeError("Invalid type cast");
              }
            },
        }, v);
      },
//      [&type](const std::shared_ptr<Variable>& arg) { return arg->type.name == type.name; },
//      [&type](const std::shared_ptr<StructObject>& arg) { return arg->type_name == type.name; },
//      [&type](const std::shared_ptr<VariantObject>& arg) { return arg->type_name == type.name; },
      [](auto) { throw RuntimeError("Invalid type comparison"); },
  }, left);
}

void Interpreter::visit(const InitalizerListExpr &expr) {

}

void Interpreter::visit(const CallExpr &expr) {

}

void Interpreter::visit(const FieldAccessExpr &expr) {

}

#pragma clang diagnostic pop
