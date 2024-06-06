#include "interpreter.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#include "utils/position.hpp"

template <typename VisitType>
typename std::enable_if<std::is_same<VisitType, Stmt>::value ||
                            std::is_same<VisitType, Expr>::value,
                        eval_value_t>::type
Interpreter::evaluate(const VisitType* visited) {
  visited->accept(*this);
  return get_evaluation();
}

template <typename VisitType>
typename std::enable_if<std::is_same<VisitType, Stmt>::value ||
                            std::is_same<VisitType, Expr>::value,
                        eval_value_t>::type
Interpreter::evaluate_var(const VisitType* visited) {
  auto var = evaluate(visited);
  while (const auto& v = std::get_if<std::shared_ptr<Variable>>(&var)) {
    var = *(v->get()->value);
  }
  return var;
}

void Interpreter::set_evaluation(eval_value_t value) {
  evaluation_ = std::move(value);
}

template <typename T>
typename std::enable_if<std::is_same_v<T, value_t>>::type
Interpreter::set_evaluation(T value) {
  set_evaluation(convert_to_eval_value(value));
}

eval_value_t Interpreter::get_evaluation() {
  assert(evaluation_.has_value());
  auto value = evaluation_;
  evaluation_.reset();
  return *value;
}

bool Interpreter::boolify(const eval_value_t& value) {
  return std::visit(overloaded{
                        [](int arg) { return arg != 0; },
                        [](float arg) { return arg != 0.F; },
                        [](const std::string& arg) { return !arg.empty(); },
                        [](bool arg) { return arg; },
                        [](auto) { return true; },
                    },
                    value);
}

void Interpreter::visit(const Program& stmt) {
  for (const auto& s : stmt.statements) {
    s->accept(*this);
  }
}

void Interpreter::visit(const PrintStmt& stmt) {
  auto value = evaluate_var(stmt.expr.get());

  std::visit(
      overloaded{
          [](auto) { throw RuntimeError("Value unprintable"); },
          [](int arg) { std::cout << std::to_string(arg); },
          [](float arg) { std::cout << std::to_string(arg); },
          [](const std::string& arg) { std::cout << arg; },
          [](bool arg) { std::cout << std::string(arg ? "true" : "false"); },
      },
      value);

  std::cout << '\n';
}

void Interpreter::visit(const LiteralExpr& expr) {
  set_evaluation(expr.literal);
}

void Interpreter::visit(const IfStmt& stmt) {
  if (boolify(evaluate(stmt.condition.get()))) {
    stmt.then_branch->accept(*this);
  } else if (stmt.else_branch) {
    stmt.else_branch->accept(*this);
  }
}

void Interpreter::visit(const BlockStmt& stmt) {
  create_new_scope();
  for (const auto& s : stmt.statements) {
    s->accept(*this);
    if (return_flag_) {
      break;
    }
  }
  pop_last_scope();
}

void Interpreter::visit(const WhileStmt& stmt) {
  while (boolify(evaluate(stmt.condition.get()))) {
    stmt.body->accept(*this);
  }
}

void Interpreter::visit(const VarDeclStmt& stmt) {
  if (const auto& var = get_variable(stmt.identifier)) {
    throw RuntimeError(stmt.position,
                       "Identifier '" + stmt.identifier + "' already defined");
  }

  auto init_value = clone_value(evaluate_var(stmt.initializer.get()));

  auto type = get_type(stmt.type.name);
  if (!type && !stmt.type.name.empty()) {
    throw RuntimeError(stmt.position,
                       "Type '" + stmt.type.name + "' not defined");
  }

  if (type) {
    std::visit(overloaded{
                   [&](const std::shared_ptr<StructType>& arg) {
                     assign_init_list(&stmt, arg, init_value);
                   },
                   [this, &stmt,
                    &init_value](const std::shared_ptr<VariantType>& arg) {
                     if (!match_type(init_value, stmt.type)) {
                       throw RuntimeError(stmt.position,
                                          "Tried to initialize '" +
                                              stmt.identifier +
                                              "' with value of different type");
                     }
                     auto obj = std::make_shared<VariantObject>(
                         arg.get(), stmt.mut, stmt.identifier, init_value);
                     define_variable(stmt.identifier, obj);
                   },
                   [&stmt](auto) {
                     throw RuntimeError(stmt.position, "Unknown type");
                   },
               },
               *type);
  } else {
    if (!match_type(init_value, stmt.type)) {
      throw RuntimeError(stmt.position, "Tried to initialize '" +
                                            stmt.identifier +
                                            "' with value of different type");
    }
    auto var = std::make_shared<Variable>(stmt.type, stmt.identifier, stmt.mut,
                                          init_value);
    define_variable(stmt.identifier, var);
  }
}

void Interpreter::visit(const StructFieldStmt&) {}

void Interpreter::visit(const StructDeclStmt& stmt) {
  if (const auto& var = get_type(stmt.identifier)) {
    throw RuntimeError(stmt.position,
                       "Type '" + stmt.identifier + "' already defined");
  }

  std::vector<Variable> vars{};
  for (const auto& field : stmt.fields) {
    vars.emplace_back(field->type, field->identifier, field->mut);
  }

  auto struct_def =
      std::make_shared<StructType>(stmt.identifier, std::move(vars));
  define_type(stmt.identifier, struct_def);
}

void Interpreter::visit(const VariantDeclStmt& stmt) {
  if (const auto& var = get_type(stmt.identifier)) {
    throw RuntimeError(stmt.position,
                       "Type '" + stmt.identifier + "' already defined");
  }

  for (const auto& param : stmt.params) {
    if (!param.name.empty() && !get_type(param.name)) {
      throw RuntimeError(stmt.position,
                         "Unknown type in variant '" + param.name + "'");
    }
  }

  auto variant_def =
      std::make_shared<VariantType>(stmt.identifier, stmt.params);
  define_type(stmt.identifier, variant_def);
}

void Interpreter::visit(const AssignStmt& stmt) {
  auto var = evaluate(stmt.var.get());
  auto value = clone_value(evaluate_var(stmt.value.get()));

  std::visit(
      overloaded{
          [this, &value](const std::shared_ptr<Variable>& arg) {
            if (!arg->mut) {
              throw RuntimeError("Tried assigning value to a const '" +
                                 arg->name + "'");
            }
            if (!match_type(value, arg->type)) {
              throw RuntimeError(
                  "Tried assigning value with different type to '" + arg->name +
                  "'");
            }
            arg->value = value;
          },
          [this, &value](const std::shared_ptr<VariantObject>& arg) {
            if (!arg->mut) {
              throw RuntimeError("Tried assigning value to a const '" +
                                 arg->name + "'");
            }
            if (!std::ranges::any_of(arg->type_def->types,
                                     [&](const VarType& param) {
                                       return match_type(value, param);
                                     })) {
              throw RuntimeError(
                  "Tried assigning value with different type to '" + arg->name +
                  "'");
            }
            arg->contained = value;
          },
          [this, &value](const std::shared_ptr<StructObject>& arg) {
            if (!arg->mut) {
              throw RuntimeError("Tried assigning value to a const '" +
                                 arg->name + "'");
            }
            if (!match_type(value,
                            VarType(arg->type_def->type_name, IDENTIFIER))) {
              throw RuntimeError(
                  "Tried assigning value with different type to '" + arg->name +
                  "'");
            }
            arg->scope = std::get<std::shared_ptr<StructObject>>(value)->scope;
          },
          [](auto) { throw RuntimeError("Invalid assignment"); },
      },
      var);
}

void Interpreter::visit(const CallStmt& stmt) {
  const auto& args = stmt.arguments;
  make_call(stmt.identifier, stmt.position, args);
}

void Interpreter::visit(const FuncParamStmt&) {}

void Interpreter::visit(const FuncStmt& stmt) {
  if (const auto& var = get_function(stmt.identifier)) {
    throw RuntimeError(stmt.position,
                       "Function '" + stmt.identifier + "' already defined");
  }
  auto* body = dynamic_cast<BlockStmt*>(stmt.body.get());
  std::vector<std::pair<std::string, VarType>> params{};
  for (const auto& param : stmt.params) {
    params.emplace_back(param->identifier, param->type);
  }
  auto func = std::make_shared<FunctionObject>(stmt.identifier,
                                               stmt.return_type, params, body);
  define_function(stmt.identifier, func);
}

void Interpreter::visit(const ReturnStmt& stmt) {
  if (stmt.value) {
    set_evaluation(evaluate_var(stmt.value.get()));
  } else {
    evaluation_.reset();
  }
  return_flag_ = true;
}

void Interpreter::visit(const LambdaFuncStmt&) {}

void Interpreter::visit(const InspectStmt& stmt) {
  auto inspected = evaluate_var(stmt.inspected.get());
  if (const auto& variant_obj =
          std::get_if<std::shared_ptr<VariantObject>>(&inspected)) {
    create_new_scope();
    for (const auto& lambda : stmt.lambdas) {
      const auto& contained = (*variant_obj)->contained;
      if (match_type(contained, lambda->type)) {
        if (auto type = get_type(lambda->type.name)) {
          std::visit(
              overloaded{
                  [&](const std::shared_ptr<StructType>& arg) {
                    const auto& struct_arg =
                        std::get<std::shared_ptr<StructObject>>(contained);
                    define_variable(lambda->identifier,
                                    std::make_shared<StructObject>(
                                        arg.get(), true, lambda->identifier,
                                        struct_arg->scope));
                  },
                  [&](const std::shared_ptr<VariantType>& arg) {
                    const auto& variant_arg =
                        std::get<std::shared_ptr<VariantObject>>(contained);
                    define_variable(lambda->identifier,
                                    std::make_shared<VariantObject>(
                                        arg.get(), true, lambda->identifier,
                                        variant_arg->contained));
                  },
                  [&](auto) {
                    throw RuntimeError(lambda->position, "Unknown type");
                  },
              },
              *type);
        } else {
          auto var = std::make_shared<Variable>(
              lambda->type, lambda->identifier, true, contained);
          define_variable(lambda->identifier, var);
        }
        lambda->body->accept(*this);
        pop_last_scope();
        return;
      }
    }
    if (!stmt.default_lambda) {
      throw RuntimeError(
          "Inspect did not match any types and default not present");
    }
    stmt.default_lambda->accept(*this);
    pop_last_scope();
  } else {
    throw RuntimeError("Cannot inspect non-variant objects");
  }
}

void Interpreter::visit(const AdditionExpr& expr) {
  perform_arithmetic_operation(expr.left.get(), expr.right.get(), std::plus<>(),
                               expr.position);
}

void Interpreter::visit(const SubtractionExpr& expr) {
  perform_arithmetic_operation(expr.left.get(), expr.right.get(),
                               std::minus<>(), expr.position);
}

void Interpreter::visit(const DivisionExpr& expr) {
  perform_arithmetic_operation(expr.left.get(), expr.right.get(),
                               std::divides<>(), expr.position);
}

void Interpreter::visit(const MultiplicationExpr& expr) {
  perform_arithmetic_operation(expr.left.get(), expr.right.get(),
                               std::multiplies<>(), expr.position);
}

void Interpreter::visit(const EqualCompExpr& expr) {
  perform_comparison_operation(expr.left.get(), expr.right.get(),
                               std::equal_to<>(), expr.position);
}

void Interpreter::visit(const NotEqualCompExpr& expr) {
  perform_comparison_operation(expr.left.get(), expr.right.get(),
                               std::not_equal_to<>(), expr.position);
}

void Interpreter::visit(const GreaterCompExpr& expr) {
  perform_comparison_operation(expr.left.get(), expr.right.get(),
                               std::greater<>(), expr.position);
}

void Interpreter::visit(const GreaterEqualCompExpr& expr) {
  perform_comparison_operation(expr.left.get(), expr.right.get(),
                               std::greater_equal<>(), expr.position);
}

void Interpreter::visit(const LessCompExpr& expr) {
  perform_comparison_operation(expr.left.get(), expr.right.get(), std::less<>(),
                               expr.position);
}

void Interpreter::visit(const LessEqualCompExpr& expr) {
  perform_comparison_operation(expr.left.get(), expr.right.get(),
                               std::less_equal<>(), expr.position);
}

void Interpreter::visit(const GroupingExpr& expr) {
  set_evaluation(evaluate(expr.expr.get()));
}

void Interpreter::visit(const NegationExpr& expr) {
  auto value = evaluate_var(expr.right.get());
  set_evaluation(boolify(value));
}

void Interpreter::visit(const LogicalNegationExpr& expr) {
  auto value = evaluate_var(expr.right.get());
  set_evaluation(!boolify(value));
}

void Interpreter::visit(const VarExpr& expr) {
  if (const auto& var = get_variable(expr.identifier)) {
    std::visit(
        overloaded{
            [this](const auto& arg) { set_evaluation(arg); },
        },
        *var);
    return;
  }
  throw RuntimeError(expr.position,
                     "Identifier '" + expr.identifier + "' not defined");
}

void Interpreter::visit(const LogicalOrExpr& expr) {
  auto right = evaluate_var(expr.right.get());
  auto left = evaluate_var(expr.left.get());
  set_evaluation(boolify(right) || boolify(left));
}

void Interpreter::visit(const LogicalAndExpr& expr) {
  auto right = evaluate_var(expr.right.get());
  auto left = evaluate_var(expr.left.get());
  set_evaluation(boolify(right) && boolify(left));
}

void Interpreter::visit(const IsTypeExpr& expr) {
  auto left = evaluate_var(expr.left.get());
  auto type = expr.type;

  set_evaluation(match_type(left, type));
}

void Interpreter::visit(const AsTypeExpr& expr) {
  auto left = evaluate_var(expr.left.get());
  auto type = expr.type;

  std::visit(overloaded{
                 [&](auto arg) {
                   if (type.type == BOOL) {
                     set_evaluation(boolify(arg));
                     return;
                   }
                   throw RuntimeError(expr.position, "Invalid type cast");
                 },
                 [&](int arg) {
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
                     case BOOL:
                       set_evaluation(boolify(arg));
                       break;
                     default:
                       set_evaluation(arg);
                       break;
                   }
                 },
                 [&](float arg) {
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
                     case BOOL:
                       set_evaluation(boolify(arg));
                       break;
                     default:
                       throw RuntimeError(expr.position, "Invalid type cast");
                   }
                 },
                 [&](std::string arg) {
                   switch (type.type) {
                     case STR:
                       set_evaluation(arg);
                       break;
                     case BOOL:
                       set_evaluation(boolify(arg));
                       break;
                     default:
                       throw RuntimeError(expr.position, "Invalid type cast");
                   }
                 },
                 [&](bool arg) {
                   switch (type.type) {
                     case STR:
                       set_evaluation(arg ? "true" : "false");
                       break;
                     case BOOL:
                       set_evaluation(arg);
                       break;
                     default:
                       throw RuntimeError(expr.position, "Invalid type cast");
                   }
                 },
                 [&](const std::shared_ptr<VariantObject>& arg) {
                   if (match_type(arg->contained, type, false)) {
                     set_evaluation(arg->contained);
                     return;
                   }
                   if (type.type == BOOL) {
                     set_evaluation(boolify(arg));
                     return;
                   }
                   throw RuntimeError(expr.position,
                                      "Invalid contained value type cast");
                 },
             },
             left);
}

void Interpreter::visit(const InitalizerListExpr& expr) {
  std::vector<eval_value_t> values{};
  for (const auto& e : expr.list) {
    values.push_back(evaluate_var(e.get()));
  }
  set_evaluation(std::make_shared<InitalizerList>(std::move(values)));
}

void Interpreter::visit(const CallExpr& expr) {
  make_call(expr.identifier, expr.position, expr.arguments);
}

void Interpreter::visit(const FieldAccessExpr& expr) {
  auto parent = evaluate(expr.parent_struct.get());
  if (const auto& struct_obj =
          std::get_if<std::shared_ptr<StructObject>>(&parent)) {
    if (const auto& eval =
            struct_obj->get()->scope.get_variable(expr.field_name)) {
      set_evaluation(*eval);
      return;
    }
    throw RuntimeError(expr.position,
                       "Field '" + expr.field_name + "' does not exist");
  }
  throw RuntimeError(expr.position,
                     "Cannot access field of a non-struct variable");
}

Scope* Interpreter::create_new_scope() {
  std::unique_ptr<Scope> new_scope;
  if (!call_contexts_.empty()) {
    new_scope =
        std::make_unique<Scope>(call_contexts_.back()->scopes.back().get());
    call_contexts_.back()->scopes.push_back(std::move(new_scope));
    return call_contexts_.back()->scopes.back().get();
  }
  new_scope = std::make_unique<Scope>(scopes_.back().get());
  scopes_.push_back(std::move(new_scope));
  return scopes_.back().get();
}

void Interpreter::pop_last_scope() {
  if (!call_contexts_.empty()) {
    call_contexts_.back()->scopes.pop_back();
  } else {
    scopes_.pop_back();
  }
}

void Interpreter::assign_init_list(const VarDeclStmt* stmt,
                                   const std::shared_ptr<StructType>& type,
                                   const eval_value_t& init_value) {
  if (const auto& init_list =
          std::get_if<std::shared_ptr<InitalizerList>>(&init_value)) {
    if (init_list->get()->values.size() != type->init_fields.size()) {
      throw RuntimeError(stmt->position,
                         "Different number of struct fields and "
                         "values in initalizer list for '" +
                             stmt->identifier + "'");
    }

    Scope struct_scope{};
    const auto& init_fields = type->init_fields;
    for (auto init_field = init_fields.rbegin();
         init_field != init_fields.rend(); ++init_field) {
      auto init_item = clone_value(init_list->get()->values.back());
      init_list->get()->values.pop_back();
      if (!match_type(init_item, init_field->type)) {
        throw RuntimeError(stmt->position,
                           "Type mismatch in initalizer list for '" +
                               stmt->identifier + "." + init_field->name + "'");
      }
      if (const auto& init_field_type = get_type(init_field->type.name)) {
        std::visit(
            overloaded{
                [&](const std::shared_ptr<VariantType>& arg) {
                  eval_value_t value = init_item;
                  if (auto* variant_obj =
                          std::get_if<std::shared_ptr<VariantObject>>(
                              &init_item)) {
                    value = (*variant_obj)->contained;
                  }
                  struct_scope.define_variable(
                      init_field->name,
                      std::make_shared<VariantObject>(
                          arg.get(), init_field->mut, init_field->name, value));
                },
                [&](const std::shared_ptr<StructType>& arg) {
                  auto struct_obj =
                      std::get<std::shared_ptr<StructObject>>(init_item);
                  struct_scope.define_variable(
                      init_field->name,
                      std::make_shared<StructObject>(arg.get(), init_field->mut,
                                                     init_field->name,
                                                     struct_obj->scope));
                },
                [&](const auto&) {
                  throw RuntimeError(stmt->position,
                                     "Unsupported type in struct declaration");
                },
            },
            *init_field_type);
      } else {
        struct_scope.define_variable(
            init_field->name,
            std::make_shared<Variable>(init_field->type, init_field->name,
                                       init_field->mut, init_item));
      }
    }
    auto obj = std::make_shared<StructObject>(
        type.get(), stmt->mut, stmt->identifier, std::move(struct_scope));
    define_variable(stmt->identifier, obj);
  } else {
    throw RuntimeError(stmt->position, "Expected initalizer list for '" +
                                           stmt->identifier + "'");
  }
}

void Interpreter::call_func(FunctionObject* func) {
  return_flag_ = false;
  for (const auto& stmt : func->body->statements) {
    stmt->accept(*this);
    if (return_flag_) {
      break;
    }
  }
  return_flag_ = false;
}

std::vector<eval_value_t> Interpreter::get_call_args_values(
    const std::vector<std::unique_ptr<Expr>>& arguments) {
  std::vector<eval_value_t> args{};
  args.reserve(arguments.size());
  for (const auto& arg : arguments) {
    args.push_back(evaluate_var(arg.get()));
  }
  return args;
}

void Interpreter::create_call_context(
    const std::shared_ptr<FunctionObject>& func, const Position& position) {
  if (call_contexts_.size() > MAX_RECURSION_DEPTH) {
    throw RuntimeError(position, "Maximum recursion depth exceeded [" +
                                     std::to_string(MAX_RECURSION_DEPTH) + "]");
  }

  auto call_context = std::make_unique<CallContext>(func);
  call_contexts_.push_back(std::move(call_context));
}

void Interpreter::pop_call_context() { call_contexts_.pop_back(); }

void Interpreter::bind_args_to_params(const FunctionObject* func,
                                      const std::vector<eval_value_t>& args,
                                      const Position& position) {
  for (size_t i = 0; i < args.size(); ++i) {
    const auto& param = func->params.at(i);
    if (!match_type(args.at(i), param.second)) {
      throw RuntimeError(position, "Type mismatch in call arguments for '" +
                                       func->identifier + "'");
    }
    if (auto type = get_type(param.second.name)) {
      std::visit(
          overloaded{
              [&](const std::shared_ptr<StructType>&) {
                auto struct_obj = std::get<std::shared_ptr<StructObject>>(
                    clone_value(args.at(i)));
                struct_obj->mut = true;
                struct_obj->name = param.first;
                define_variable(param.first, struct_obj);
              },
              [&](const std::shared_ptr<VariantType>&) {
                auto variant_obj = std::get<std::shared_ptr<VariantObject>>(
                    clone_value(args.at(i)));
                variant_obj->mut = true;
                variant_obj->name = param.first;
                define_variable(param.first, variant_obj);
              },
              [&](auto) { throw RuntimeError(position, "Unknown type"); },
          },
          *type);
    } else {
      auto var = std::make_shared<Variable>(param.second.type, param.first,
                                            true, clone_value(args.at(i)));
      define_variable(param.first, var);
    }
  }
}

void Interpreter::make_call(
    const std::string& identifier, const Position& position,
    const std::vector<std::unique_ptr<Expr>>& arguments) {
  auto opt_func = get_function(identifier);
  if (!opt_func) {
    throw RuntimeError(position, "Function '" + identifier + "' not defined");
  }

  auto func = *opt_func;

  auto args = get_call_args_values(arguments);
  if (args.size() != func->params.size()) {
    throw RuntimeError(
        position, "Invalid number of arguments in '" + identifier + "' call");
  }

  create_call_context(func, position);
  bind_args_to_params(func.get(), args, position);
  call_func(func.get());

  if (func->return_type.type == VOID) {
    if (evaluation_) {
      throw RuntimeError(position, "Void function returned a value");
    }
  } else {
    if (!evaluation_) {
      throw RuntimeError(position, "Non-void function did not return a value");
    }
    if (!match_type(*evaluation_, func->return_type)) {
      throw RuntimeError(
          position,
          "Function returned value with different type than declared");
    }
  }

  pop_call_context();
}

void Interpreter::define_variable(const std::string& name,
                                  const eval_value_t& variable) {
  if (!call_contexts_.empty()) {
    call_contexts_.back()->scopes.back()->define_variable(name, variable);
  } else {
    scopes_.back()->define_variable(name, variable);
  }
}

void Interpreter::define_type(const std::string& name, const types_t& type) {
  if (!call_contexts_.empty()) {
    call_contexts_.back()->scopes.back()->define_type(name, type);
  } else {
    scopes_.back()->define_type(name, type);
  }
}

void Interpreter::define_function(const std::string& name,
                                  const function_t& function) {
  if (!call_contexts_.empty()) {
    call_contexts_.back()->scopes.back()->define_function(name, function);
  } else {
    scopes_.back()->define_function(name, function);
  }
}

std::optional<eval_value_t> Interpreter::get_variable(
    const std::string& name) const {
  if (!call_contexts_.empty()) {
    if (auto variable =
            call_contexts_.back()->scopes.back()->get_variable(name)) {
      return variable;
    }
  }
  return scopes_.back()->get_variable(name);
}

std::optional<types_t> Interpreter::get_type(const std::string& name) const {
  if (!call_contexts_.empty()) {
    if (auto type = call_contexts_.back()->scopes.back()->get_type(name)) {
      return type;
    }
  }
  return scopes_.back()->get_type(name);
}

std::optional<function_t> Interpreter::get_function(
    const std::string& name) const {
  if (!call_contexts_.empty()) {
    if (auto func = call_contexts_.back()->scopes.back()->get_function(name)) {
      return func;
    }
  }
  return scopes_.back()->get_function(name);
}

bool Interpreter::match_type(const eval_value_t& actual,
                             const VarType& expected, bool check_self) const {
  if (!call_contexts_.empty()) {
    if (auto match = call_contexts_.back()->scopes.back()->match_type(
            actual, expected, check_self)) {
      return match;
    }
  }
  return scopes_.back()->match_type(actual, expected, check_self);
}

template <typename Operation>
void Interpreter::perform_arithmetic_operation(Expr* left, Expr* right,
                                               Operation op,
                                               const Position& position) {
  auto leftValue = evaluate_var(left);
  auto rightValue = evaluate_var(right);

  std::visit(
      overloaded{
          [&](int lhs, int rhs) { set_evaluation(op(lhs, rhs)); },
          [&](float lhs, float rhs) { set_evaluation(op(lhs, rhs)); },
          [&](const std::string& lhs, const std::string& rhs) {
            if constexpr (std::is_same_v<Operation, std::plus<>>) {
              set_evaluation(lhs + rhs);
            } else {
              throw RuntimeError(position, "Unsupported operation for strings");
            }
          },
          [&](auto lhs, auto rhs) {
            if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>) {
              throw RuntimeError(position,
                                 "Unsupported types for arithmetic operation");
            }
            throw RuntimeError(
                position,
                "Arithmetic operation cannot be applied to different types");
          },
      },
      leftValue, rightValue);
}

template <typename Operation>
void Interpreter::perform_comparison_operation(Expr* left, Expr* right,
                                               Operation op,
                                               const Position& position) {
  auto leftValue = evaluate_var(left);
  auto rightValue = evaluate_var(right);

  std::visit(
      overloaded{
          [&](int lhs, int rhs) { set_evaluation(op(lhs, rhs)); },
          [&](float lhs, float rhs) { set_evaluation(op(lhs, rhs)); },
          [&](bool lhs, bool rhs) { set_evaluation(op(lhs, rhs)); },
          [&](const std::string& lhs, const std::string& rhs) {
            set_evaluation(op(lhs, rhs));
          },
          [&](auto lhs, auto rhs) {
            if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>) {
              throw RuntimeError(position,
                                 "Unsupported types for comparison operation");
            }
            throw RuntimeError(
                position,
                "Comparison operation cannot be applied to different types");
          },
      },
      leftValue, rightValue);
}
