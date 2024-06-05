#include "scope.hpp"

#include <algorithm>

bool Scope::type_in_variant(const std::vector<VarType>& variant_types,
                            BuiltinType type) {
  return std::ranges::any_of(variant_types, [&type](const VarType& param) {
    return param.type == type;
  });
}

bool Scope::identifier_in_variant(const std::vector<VarType>& variant_types,
                                  const std::string& identifier) const {
  if (!get_type(identifier)) {
    return false;
  }

  return std::ranges::any_of(
      variant_types, [&identifier](const VarType& param) {
        return param.type == IDENTIFIER && param.name == identifier;
      });
}

bool Scope::match_type(const eval_value_t& actual, const VarType& expected,
                       bool check_self) const {
  if (is_in_variant(actual, expected, check_self)) {
    return true;
  }

  std::function<bool(const eval_value_t&)> check =
      [&](const auto& arg) -> bool {
    return std::visit(
        overloaded{
            [&expected](int) { return expected.type == INT; },
            [&expected](float) { return expected.type == FLOAT; },
            [&expected](const std::string&) { return expected.type == STR; },
            [&expected](bool) { return expected.type == BOOL; },
            [&expected](const std::shared_ptr<Variable>& arg) {
              return arg->type.name == expected.name;
            },
            [&expected](const std::shared_ptr<StructObject>& arg) {
              return arg->type_def->type_name == expected.name;
            },
            [&](const std::shared_ptr<VariantObject>& arg) {
              return arg->type_def->type_name == expected.name ||
                     check(arg->contained);
            },
            [](auto) { return false; }},
        arg);
  };

  return check(actual);
}

void Scope::define_variable(const std::string& name, eval_value_t variable) {
  variables_.insert({name, std::move(variable)});
}

const std::map<std::string, eval_value_t>& Scope::get_variables() const {
  return variables_;
}

std::optional<eval_value_t> Scope::get_variable(const std::string& name) const {
  auto item = variables_.find(name);
  if (item != variables_.end()) {
    return item->second;
  }
  if (enclosing_ != nullptr) {
    return enclosing_->get_variable(name);
  }
  return std::nullopt;
}

std::optional<types_t> Scope::get_type(const std::string& name) const {
  auto item = types_.find(name);
  if (item != types_.end()) {
    return item->second;
  }
  if (enclosing_ != nullptr) {
    return enclosing_->get_type(name);
  }
  return std::nullopt;
}

std::optional<function_t> Scope::get_function(const std::string& name) const {
  auto item = functions_.find(name);
  if (item != functions_.end()) {
    return item->second;
  }
  if (enclosing_ != nullptr) {
    return enclosing_->get_function(name);
  }
  return std::nullopt;
}

void Scope::define_type(const std::string& name, types_t type) {
  types_.insert({name, std::move(type)});
}

void Scope::define_function(const std::string& name, function_t function) {
  functions_.insert({name, std::move(function)});
}
bool Scope::is_in_variant(const eval_value_t& actual, const VarType& expected,
                          bool check_self) const {
  if (check_self && !expected.name.empty() && expected.type == IDENTIFIER) {
    if (const auto& type = get_type(expected.name)) {
      if (const auto& variant =
              std::get_if<std::shared_ptr<VariantType>>(&*type)) {
        return std::visit(
            overloaded{[&](std::monostate) {
                         return type_in_variant(variant->get()->types, VOID);
                       },
                       [&](const std::string&) {
                         return type_in_variant(variant->get()->types, STR);
                       },
                       [&](int) {
                         return type_in_variant(variant->get()->types, INT);
                       },
                       [&](float) {
                         return type_in_variant(variant->get()->types, FLOAT);
                       },
                       [&](bool) {
                         return type_in_variant(variant->get()->types, BOOL);
                       },
                       [&](const std::shared_ptr<Variable>& obj) {
                         return identifier_in_variant(variant->get()->types,
                                                      obj->type.name);
                       },
                       [&](const std::shared_ptr<StructObject>& obj) {
                         return identifier_in_variant(variant->get()->types,
                                                      obj->type_def->type_name);
                       },
                       [&](const std::shared_ptr<VariantObject>& obj) {
                         return identifier_in_variant(
                                    variant->get()->types,
                                    obj->type_def->type_name) ||
                                obj->type_def->type_name == expected.name;
                       },
                       [](auto) { return false; }},
            actual);
      }
    }
  }
  return false;
}

Variable Variable::clone() const {
  if (value) {
    return {type, name, mut, clone_value(*value)};
  }
  return {type, name, mut};
}

VariantObject VariantObject::clone() const {
  return {type_def, mut, name, clone_value(contained)};
}

StructObject StructObject::clone() const {
  return {type_def, mut, name, clone_scope()};
}

Scope StructObject::clone_scope() const {
  Scope new_scope;
  const auto& variables = scope.get_variables();

  std::for_each(
      variables.begin(), variables.end(), [&new_scope](const auto& pair) {
        new_scope.define_variable(pair.first, clone_value(pair.second));
      });
  return new_scope;
}

eval_value_t clone_value(const eval_value_t& value) {
  eval_value_t cloned;
  std::visit(overloaded{[&](const std::shared_ptr<Variable>& obj) {
                          cloned = std::make_shared<Variable>(obj->clone());
                        },
                        [&](const std::shared_ptr<StructObject>& obj) {
                          cloned = std::make_shared<StructObject>(obj->clone());
                        },
                        [&](const std::shared_ptr<VariantObject>& obj) {
                          cloned =
                              std::make_shared<VariantObject>(obj->clone());
                        },
                        [&](auto arg) { cloned = arg; }},
             value);
  return cloned;
}

eval_value_t convert_to_eval_value(const value_t& value) {
  return std::visit([](auto&& arg) -> eval_value_t { return arg; }, value);
}
