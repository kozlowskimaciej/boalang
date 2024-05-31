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

template <typename VisitType>
eval_value_t Interpreter::evaluate(const VisitType* visited) {
  visited->accept(*this);
  return get_evaluation();
}

template <typename VisitType>
eval_value_t Interpreter::evaluate_var(const VisitType *visited) {
  auto var = evaluate(visited);
  while (const auto& v = std::get_if<std::shared_ptr<Variable>>(&var)) {
    var = *(v->get()->value);
  }
  return var;
}

void Interpreter::set_evaluation(eval_value_t value) {
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
  std::unique_ptr<Scope> new_scope;
  if (!call_contexts.empty()) {
    new_scope = std::make_unique<Scope>(call_contexts.back()->scopes.back().get());
    call_contexts.back()->scopes.push_back(std::move(new_scope));
  } else {
    new_scope = std::make_unique<Scope>(scopes.back().get());
    scopes.push_back(std::move(new_scope));
  }
  std::exception_ptr eptr = nullptr;
  try {
    for (const auto& s : stmt.statements) {
      if (return_flag) {
        return_flag = false;
        break;
      }
      s->accept(*this);
    }
  } catch(...) {
    eptr = std::current_exception();
  }
  if (!call_contexts.empty()) {
    call_contexts.back()->scopes.pop_back();
  } else {
    scopes.pop_back();
  }
  if (eptr) { std::rethrow_exception(eptr); }
}

void Interpreter::visit(const WhileStmt &stmt) {
  while (boolify(evaluate(stmt.condition.get()))) {
    stmt.body->accept(*this);
  }
}

void Interpreter::visit(const VarDeclStmt &stmt) {
  if (const auto& var = get_variable(stmt.identifier)) {
    throw RuntimeError(stmt.position, "Identifier '" + stmt.identifier + "' already defined");
  }

  auto init_value = evaluate_var(stmt.initializer.get());

  auto type = get_type(stmt.type.name);
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
              if (!match_type(init_item, field->type)) {
                throw RuntimeError(stmt.position, "Type mismatch in initalizer list for '" + stmt.identifier + "." + field->name + "'");
              }
              if (const auto& type = get_type(field->type.name)) {
                std::visit(overloaded{
                    [&](const std::shared_ptr<VariantType>& arg) {
                      struct_scope.define_variable(field->name, std::make_shared<VariantObject>(arg.get(), field->mut, field->name, init_item));
                    },
                    [&](const std::shared_ptr<StructType>& arg) {
                      auto struct_obj = std::get<std::shared_ptr<StructObject>>(init_item);
                      struct_scope.define_variable(field->name, std::make_shared<StructObject>(arg.get(), field->name, struct_obj->scope));
                    },
                    [&](const auto& arg) {
                      throw RuntimeError(stmt.position, "Unsupported type in struct declaration");
                    },
                }, *type);
              } else {
                struct_scope.define_variable(field->name, std::make_shared<Variable>(field->type, field->name, field->mut, init_item));
              }
            }
            auto obj = std::make_shared<StructObject>(arg.get(), stmt.identifier, std::move(struct_scope));
            define_variable(stmt.identifier, obj);
          } else {
            throw RuntimeError(stmt.position, "Expected initalizer list for '" + stmt.identifier + "'");
          }
        },
        [this, &stmt, &init_value](const std::shared_ptr<VariantType>& arg) {
          if (!match_type(init_value, stmt.type)) {
            throw RuntimeError(stmt.position, "Tried to initialize '" + stmt.identifier + "' with value of different type");
          }
          auto obj = std::make_shared<VariantObject>(arg.get(), stmt.mut, stmt.identifier, init_value);
          define_variable(stmt.identifier, obj);
        },
        [&stmt](auto) { throw RuntimeError(stmt.position, "Unknown type"); },
    }, *type);
  } else {
    if (!match_type(init_value, stmt.type)) {
      throw RuntimeError(stmt.position, "Tried to initialize '" + stmt.identifier + "' with value of different type");
    }
    auto var = std::make_shared<Variable>(stmt.type, stmt.identifier, stmt.mut, init_value);
    define_variable(stmt.identifier, var);
  }
}

void Interpreter::visit(const StructFieldStmt &stmt) {

}

void Interpreter::visit(const StructDeclStmt &stmt) {
  if (const auto& var = get_type(stmt.identifier)) {
    throw RuntimeError(stmt.position, "Type '" + stmt.identifier + "' already defined");
  }

  std::vector<Variable> vars{};
  for (const auto& field : stmt.fields) {
    vars.emplace_back(field->type, field->identifier, field->mut);
  }

  auto struct_def = std::make_shared<StructType>(stmt.identifier, std::move(vars));
  define_type(stmt.identifier, struct_def);
}

void Interpreter::visit(const VariantDeclStmt &stmt) {
  if (const auto& var = get_type(stmt.identifier)) {
    throw RuntimeError(stmt.position, "Type '" + stmt.identifier + "' already defined");
  }

  for (const auto& param : stmt.params) {
    if (!param.name.empty() && !get_type(param.name)) {
      throw RuntimeError(stmt.position, "Unknown type in variant '" + param.name + "'");
    }
  }

  auto variant_def = std::make_shared<VariantType>(stmt.identifier, stmt.params);
  define_type(stmt.identifier, variant_def);
}

void Interpreter::visit(const AssignStmt &stmt) {
  auto var = evaluate(stmt.var.get());
  auto value = evaluate_var(stmt.value.get());

  std::visit(overloaded{
      [this, &value](const std::shared_ptr<Variable>& arg) {
        if (!arg->mut) {
          throw RuntimeError("Tried assigning value to a const '" + arg->name + "'");
        }
        if (!match_type(value, arg->type)) {
          throw RuntimeError("Tried assigning value with different type to '" + arg->name + "'");
        }
        arg->value = std::move(value);
      },
      [this, &value](const std::shared_ptr<VariantObject>& arg) {
        if (!arg->mut) {
          throw RuntimeError("Tried assigning value to a const '" + arg->name + "'");
        }
        if (!std::ranges::any_of(arg->type_def->types, [&](const VarType& param) { return match_type(value, param); } )){
            throw RuntimeError("Tried assigning value with different type to '" + arg->name + "'");
        }
        arg->contained = std::move(value);
      },
      [](auto) { throw RuntimeError("Invalid assignment"); },
  }, var);
}

void Interpreter::visit(const CallStmt &stmt) {
  auto& args = stmt.arguments;
  make_call(stmt.identifier, stmt.position, args);
}

void Interpreter::visit(const FuncParamStmt &stmt) {
//  if (auto type = get_type(stmt.type.name)) {
//    std::visit(overloaded{
//        [&](const std::shared_ptr<StructType>& arg) {
//          auto struct_obj = std::make_shared<StructObject>(arg.get(), stmt.identifier, Scope());
//          set_evaluation(struct_obj);
//        },
//        [&](const std::shared_ptr<VariantType>& arg) {
//          auto variant_obj = std::make_shared<VariantObject>(arg.get(), true, stmt.identifier, std::monostate());
//          set_evaluation(variant_obj);
//        },
//        [&stmt](auto) { throw RuntimeError(stmt.position, "Unknown type"); },
//    }, *type);
//  } else {
//    auto var = std::make_shared<Variable>(stmt.type, stmt.identifier, true);
//    set_evaluation(var);
//  }
}

void Interpreter::visit(const FuncStmt &stmt) {
  auto* body = dynamic_cast<BlockStmt*>(stmt.body.get());
  std::vector<std::pair<std::string, VarType>> params {};
  for (const auto& param : stmt.params) {
    params.emplace_back(param->identifier, param->type);
  }
  auto func = std::make_shared<FunctionObject>(stmt.identifier, stmt.return_type, params, body);
  define_function(stmt.identifier, func);
}

void Interpreter::visit(const ReturnStmt &stmt) {
  if (stmt.value) {
    set_evaluation(evaluate_var(stmt.value.get()));
  } else {
    evaluation.reset();
  }
  return_flag = true;
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
  if (const auto& var = get_variable(expr.identifier)) {
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

  set_evaluation(match_type(left, type));
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
        if (match_type(arg->contained, type, false)) {
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
  make_call(expr.identifier, expr.position, expr.arguments);
}

void Interpreter::visit(const FieldAccessExpr &expr) {
  auto parent = evaluate(expr.parent_struct.get());
  if (const auto& struct_obj = std::get_if<std::shared_ptr<StructObject>>(&parent)) {
    if (const auto& eval = struct_obj->get()->scope.get_variable(expr.field_name)) {
      set_evaluation(*eval);
      return;
    }
    throw RuntimeError(expr.position, "Field '" + expr.field_name +"' does not exist");
  }
  throw RuntimeError(expr.position, "Cannot access field of a non-struct variable");
}

void Interpreter::call_func(FunctionObject *func) {
  return_flag = false;
  for (const auto& stmt : func->body->statements) {
    stmt->accept(*this);
    if (return_flag) {
      break;
    }
  }
}

std::vector<eval_value_t> Interpreter::get_call_args_values(const std::vector<std::unique_ptr<Expr>> &arguments) {
  std::vector<eval_value_t> args{};
  for (const auto& arg : arguments) {
    args.push_back(evaluate_var(arg.get()));
  }
  return args;
}

void Interpreter::create_call_context(std::shared_ptr<FunctionObject> func, const Position &position) {
  ++CallContext::nested;
  if (CallContext::nested > MAX_RECURSION_DEPTH) {
    throw RuntimeError(position, "Maximum recursion depth exceeded [" + std::to_string(MAX_RECURSION_DEPTH) + "]");
  }

  auto call_context = std::make_unique<CallContext>(func);
  call_contexts.push_back(std::move(call_context));
}

void Interpreter::pop_call_context() {
  call_contexts.pop_back();
  --CallContext::nested;
}

void Interpreter::bind_args_to_params(const FunctionObject *func, const std::vector<eval_value_t>& args, const Position& position) {
  for (size_t i = 0; i < args.size(); ++i) {
    const auto& param = func->params.at(i);
    if (!match_type(args.at(i), param.second)) {
      throw RuntimeError(position, "Type mismatch in call arguments for '" + func->identifier + "'");
    }
    if (auto type = get_type(param.second.name)) {
      std::visit(overloaded{
          [&](const std::shared_ptr<StructType>& arg) {
            const auto& struct_arg = std::get<std::shared_ptr<StructObject>>(args.at(i));
            auto struct_obj = std::make_shared<StructObject>(arg.get(), param.first, struct_arg->scope);
            define_variable(param.first, struct_obj);
          },
          [&](const std::shared_ptr<VariantType>& arg) {
            auto variant_obj = std::make_shared<VariantObject>(arg.get(), true, param.first, args.at(i));
            define_variable(param.first, variant_obj);
          },
          [&](auto) { throw RuntimeError(position, "Unknown type"); },
      }, *type);
    } else {
      auto var = std::make_shared<Variable>(param.second.type, param.first, true, args.at(i));
      define_variable(param.first, var);
    }
  }
}

void Interpreter::make_call(const std::string &identifier, const Position &position, const std::vector<std::unique_ptr<Expr>>& arguments) {
  auto opt_func = get_function(identifier);
  if (!opt_func) {
    throw RuntimeError(position, "Function '" + identifier + "' not defined");
  }

  auto func = *opt_func;

  auto args = get_call_args_values(arguments);
  if (args.size() != func->params.size()) {
    throw RuntimeError(position, "Invalid number of arguments in '" + identifier + "' call");
  }

  create_call_context(func, position);
  bind_args_to_params(func.get(), args, position);
  call_func(func.get());

  if (!evaluation && func->return_type.type != VOID) {
    throw RuntimeError(position, "Non-void function did not return a value");
  }
  if (!match_type(*evaluation, func->return_type)) {
    throw RuntimeError(position, "Function returned value with different type than declared");
  }
  pop_call_context();
}

void Interpreter::define_variable(const std::string &name, const eval_value_t &variable) {
  if (!call_contexts.empty()) {
    call_contexts.back()->scopes.back()->define_variable(name, variable);
  } else {
    scopes.back()->define_variable(name, variable);
  }
}

void Interpreter::define_type(const std::string &name, const types_t &type) {
  if (!call_contexts.empty()) {
    call_contexts.back()->scopes.back()->define_type(name, type);
  } else {
    scopes.back()->define_type(name, type);
  }
}

void Interpreter::define_function(const std::string &name, const function_t &function) {
  if (!call_contexts.empty()) {
    call_contexts.back()->scopes.back()->define_function(name, function);
  } else {
    scopes.back()->define_function(name, function);
  }
}

std::optional<eval_value_t> Interpreter::get_variable(const std::string &name) const {
  if (!call_contexts.empty()) {
    if (auto variable = call_contexts.back()->scopes.back()->get_variable(name)) {
      return variable;
    }
  }
  return scopes.back()->get_variable(name);
}

std::optional<types_t> Interpreter::get_type(const std::string &name) const {
  if (!call_contexts.empty()) {
    if (auto type = call_contexts.back()->scopes.back()->get_type(name)) {
      return type;
    }
  }
  return scopes.back()->get_type(name);
}

std::optional<function_t> Interpreter::get_function(const std::string &name) const {
  if (!call_contexts.empty()) {
    if (auto func = call_contexts.back()->scopes.back()->get_function(name)) {
      return func;
    }
  }
  return scopes.back()->get_function(name);
}

bool Interpreter::match_type(const eval_value_t &actual, const VarType &expected, bool check_self) const {
  if (!call_contexts.empty()) {
    if (auto match = call_contexts.back()->scopes.back()->match_type(actual, expected, check_self)) {
      return match;
    }
  }
  return scopes.back()->match_type(actual, expected, check_self);
}

#pragma clang diagnostic pop
