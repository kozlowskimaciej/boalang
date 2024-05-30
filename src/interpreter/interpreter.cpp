#include "interpreter.hpp"

#include <algorithm>
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

eval_value_t Interpreter::evaluate_var(const Expr *expr) {
  auto var = evaluate(expr);
  while (const auto& v = std::get_if<std::shared_ptr<Variable>>(&var)) {
    var = *(v->get()->value);
  }
  return var;
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
  auto value = evaluate_var(stmt.expr.get());

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
      [](auto) { throw RuntimeError("Value unprintable"); },
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
  auto new_scope = std::make_unique<Scope>(scopes.back().get());
  scopes.push_back(std::move(new_scope));
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
  if (const auto& var = scopes.back()->get(stmt.identifier)) {
    throw RuntimeError(stmt.position, "Identifier '" + stmt.identifier + "' already defined");
  }

  auto init_value = evaluate_var(stmt.initializer.get());

  auto type = scopes.back()->get_type(stmt.type.name);
  if (!type && !stmt.type.name.empty()) {
    throw RuntimeError(stmt.position, "Type '" + stmt.type.name + "' is not defined");
  }

  if (type) {
    std::visit(overloaded{
        [&](const std::shared_ptr<StructType>& arg) {
          if (const auto& init_list = std::get_if<std::shared_ptr<InitalizerList>>(&init_value)) {
            if (init_list->get()->values.size() != arg->init_fields.size()) {
              throw RuntimeError(stmt.position, "Different number of struct fields and values in initalizer list for '" + stmt.identifier + "'");
            }

            Scope struct_scope{};
            const auto& init_fields = arg->init_fields;
            for (auto field = init_fields.rbegin(); field != init_fields.rend(); ++field) {
              auto init_item = init_list->get()->values.back();
              if (!scopes.back()->match_type(init_item, field->type)) {
                throw RuntimeError(stmt.position, "Type mismatch in initalizer list for '" + stmt.identifier + "." + field->name + "'");
              }
              if (const auto& type = scopes.back()->get_type(field->type.name)) {
                std::visit(overloaded{
                    [&](const std::shared_ptr<VariantType>& arg) {
                      struct_scope.define(field->name, std::make_shared<VariantObject>(arg.get(), field->mut, field->name, init_item));
                    },
                    [&](const std::shared_ptr<StructType>& arg) {
                      auto struct_obj = std::get<std::shared_ptr<StructObject>>(init_item);
                      struct_scope.define(field->name, std::make_shared<StructObject>(arg.get(), field->name, struct_obj->scope));
                    },
                    [&](const auto& arg) {
                      throw RuntimeError(stmt.position, "Unsupported type in struct declaration");
                    },
                }, *type);
              } else {
                struct_scope.define(field->name, std::make_shared<Variable>(field->type, field->name, field->mut, init_item));
              }
            }
            auto obj = std::make_shared<StructObject>(arg.get(), stmt.identifier, std::move(struct_scope));
            scopes.back()->define(stmt.identifier, obj);
          } else {
            throw RuntimeError(stmt.position, "Expected initalizer list for '" + stmt.identifier + "'");
          }
        },
        [this, &stmt, &init_value](const std::shared_ptr<VariantType>& arg) {
          if (!scopes.back()->match_type(init_value, stmt.type)) {
            throw RuntimeError(stmt.position, "Tried to initialize '" + stmt.identifier + "' with value of different type");
          }
          auto obj = std::make_shared<VariantObject>(arg.get(), stmt.mut, stmt.identifier, init_value);
          scopes.back()->define(stmt.identifier, obj);
        },
        [&stmt](auto) { throw RuntimeError(stmt.position, "Unknown type"); },
    }, *type);
  } else {
    if (!scopes.back()->match_type(init_value, stmt.type)) {
      throw RuntimeError(stmt.position, "Tried to initialize '" + stmt.identifier + "' with value of different type");
    }
    auto var = std::make_shared<Variable>(stmt.type, stmt.identifier, stmt.mut, init_value);
    scopes.back()->define(stmt.identifier, var);
  }
}

void Interpreter::visit(const StructFieldStmt &stmt) {

}

void Interpreter::visit(const StructDeclStmt &stmt) {
  if (const auto& var = scopes.back()->get_type(stmt.identifier)) {
    throw RuntimeError(stmt.position, "Type '" + stmt.identifier + "' already defined");
  }

  std::vector<Variable> vars{};
  for (const auto& field : stmt.fields) {
    vars.emplace_back(field->type, field->identifier, field->mut);
  }

  auto struct_def = std::make_shared<StructType>(stmt.identifier, std::move(vars));
  scopes.back()->define_type(stmt.identifier, struct_def);
}

void Interpreter::visit(const VariantDeclStmt &stmt) {
  if (const auto& var = scopes.back()->get_type(stmt.identifier)) {
    throw RuntimeError(stmt.position, "Type '" + stmt.identifier + "' already defined");
  }

  for (const auto& param : stmt.params) {
    if (!param.name.empty() && !scopes.back()->get_type(param.name)) {
      throw RuntimeError(stmt.position, "Unknown type in variant '" + param.name + "'");
    }
  }

  auto variant_def = std::make_shared<VariantType>(stmt.identifier, stmt.params);
  scopes.back()->define_type(stmt.identifier, variant_def);
}

void Interpreter::visit(const AssignStmt &stmt) {
  auto var = evaluate(stmt.var.get());
  auto value = evaluate_var(stmt.value.get());

  std::visit(overloaded{
      [this, &value](const std::shared_ptr<Variable>& arg) {
        if (!arg->mut) {
          throw RuntimeError("Tried assigning value to a const '" + arg->name + "'");
        }
        if (!scopes.back()->match_type(value, arg->type)) {
          throw RuntimeError("Tried assigning value with different type to '" + arg->name + "'");
        }
        arg->value = std::move(value);
//        scopes.back()->assign(arg->name, value);
      },
      [this, &value](const std::shared_ptr<VariantObject>& arg) {
        if (!arg->mut) {
          throw RuntimeError("Tried assigning value to a const '" + arg->name + "'");
        }
        if (!std::ranges::any_of(arg->type_def->types, [&](const VarType& param) { return scopes.back()->match_type(value, param); } )){
            throw RuntimeError("Tried assigning value with different type to '" + arg->name + "'");
        }
        arg->contained = std::move(value);
      },
      [](auto) { throw RuntimeError("Invalid assignment"); },
  }, var);
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
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
    [this, &expr](const value_t& lhs, const value_t& rhs) {
      std::visit(overloaded{
        [this](int lhs, int rhs) { set_evaluation(lhs + rhs); },
        [this](float lhs, float rhs) { set_evaluation(lhs + rhs); },
        [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs + rhs); },
        [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot add different types"); }
      }, lhs, rhs);
    },
    [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for addition"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const SubtractionExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
    [this, &expr](const value_t& lhs, const value_t& rhs) {
      std::visit(overloaded{
          [this](int lhs, int rhs) { set_evaluation(lhs - rhs); },
          [this](float lhs, float rhs) { set_evaluation(lhs - rhs); },
          [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot substract different types"); }
      }, lhs, rhs);
    },
    [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for subtraction"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const DivisionExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
      [this, &expr](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs / rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs / rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot divide different types"); }
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for division"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const MultiplicationExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
      [this, &expr](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs * rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs * rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot multiply different types"); }
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for multiplication"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const EqualCompExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

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
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for comparison"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const NotEqualCompExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

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
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for comparison"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const GreaterCompExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs > rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs > rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs > rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs > rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare different types"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for comparison"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const GreaterEqualCompExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs >= rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs >= rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs >= rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs >= rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare different types"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for comparison"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const LessCompExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs < rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs < rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs < rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs < rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare different types"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for comparison"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const LessEqualCompExpr &expr) {
  auto leftValue = evaluate_var(expr.left.get());
  auto rightValue = evaluate_var(expr.right.get());

  std::visit(overloaded{
      [&expr, this](const value_t& lhs, const value_t& rhs) {
        std::visit(overloaded{
            [this](int lhs, int rhs) { set_evaluation(lhs <= rhs); },
            [this](float lhs, float rhs) { set_evaluation(lhs <= rhs); },
            [this](bool lhs, bool rhs) { set_evaluation(lhs <= rhs); },
            [this](const std::string& lhs, const std::string& rhs) { set_evaluation(lhs <= rhs); },
            [&expr](auto, auto) { throw RuntimeError(expr.position, "Cannot compare different types"); },
        }, lhs, rhs);
      },
      [&expr](auto, auto) { throw RuntimeError(expr.position, "Unsupported types for comparison"); },
  }, leftValue, rightValue);
}

void Interpreter::visit(const GroupingExpr &expr) {
  set_evaluation(evaluate(expr.expr.get()));
}

void Interpreter::visit(const NegationExpr &expr) {
  auto value = evaluate_var(expr.right.get());
  set_evaluation(boolify(value));
}

void Interpreter::visit(const LogicalNegationExpr &expr) {
  auto value = evaluate_var(expr.right.get());
  set_evaluation(!boolify(value));
}

void Interpreter::visit(const VarExpr &expr) {
  if (const auto& var = scopes.back()->get(expr.identifier)) {
    std::visit(overloaded{
      [this](const auto& arg) { set_evaluation(arg); },
    }, *var);
    return;
  }
  throw RuntimeError(expr.position, "Identifier '" + expr.identifier + "' not found");
}

void Interpreter::visit(const LogicalOrExpr &expr) {
  auto right = evaluate_var(expr.right.get());
  auto left = evaluate_var(expr.left.get());
  set_evaluation(boolify(right) || boolify(left));
}

void Interpreter::visit(const LogicalAndExpr &expr) {
  auto right = evaluate_var(expr.right.get());
  auto left = evaluate_var(expr.left.get());
  set_evaluation(boolify(right) && boolify(left));
}

void Interpreter::visit(const IsTypeExpr &expr) {
  auto left = evaluate_var(expr.left.get());
  auto type = expr.type;

  set_evaluation(scopes.back()->match_type(left, type));
}

void Interpreter::visit(const AsTypeExpr &expr) {
  auto left = evaluate_var(expr.left.get());
  auto type = expr.type;

  if (type.type == BOOL) {
    set_evaluation(boolify(left));
    return;
  }

  std::visit(overloaded{
      [this, &expr, &type](const value_t& v) {
        std::visit(overloaded{
            [&expr](auto) { throw RuntimeError(expr.position, "Invalid type cast"); },
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
            [this, &expr, &type](float arg) {
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
                  throw RuntimeError(expr.position, "Invalid type cast");
              }
            },
            [this, &expr, &type](std::string arg) {
              switch (type.type) {
                case STR:
                  set_evaluation(arg);
                  break;
                default:
                  throw RuntimeError(expr.position, "Invalid type cast");
              }
            },
            [this, &expr, &type](bool arg) {
              switch (type.type) {
                case STR:
                  set_evaluation(arg?"true":"false");
                  break;
                default:
                  throw RuntimeError(expr.position, "Invalid type cast");
              }
            },
        }, v);
      },
//      [&type](const std::shared_ptr<StructObject>& arg) { return arg->type_name == type.name; },
      [&](const std::shared_ptr<VariantObject>& arg) {
        if (scopes.back()->match_type(arg->contained, type, false)) {
          set_evaluation(arg->contained);
          return;
        }
        throw RuntimeError(expr.position, "Invalid contained value type cast");
      },
      [&expr](auto) { throw RuntimeError(expr.position, "Invalid type cast"); },
  }, left);
}

void Interpreter::visit(const InitalizerListExpr &expr) {
  std::vector<eval_value_t> values{};
  for (const auto& e : expr.list) {
    values.push_back(evaluate_var(e.get()));
  }
  set_evaluation(std::make_shared<InitalizerList>(std::move(values)));
}

void Interpreter::visit(const CallExpr &expr) {

}

void Interpreter::visit(const FieldAccessExpr &expr) {
  auto parent = evaluate(expr.parent_struct.get());
  if (const auto& struct_obj = std::get_if<std::shared_ptr<StructObject>>(&parent)) {
    if (const auto& eval = struct_obj->get()->scope.get(expr.field_name)) {
      set_evaluation(*eval);
      return;
    }
    throw RuntimeError(expr.position, "Field '" + expr.field_name +"' does not exist");
  }
  throw RuntimeError(expr.position, "Cannot access field of a non-struct variable");
}

#pragma clang diagnostic pop
