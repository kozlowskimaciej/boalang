#include "scope.hpp"

#include <algorithm>

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

bool Scope::type_in_variant(const std::vector<VarType>& variant_types, BuiltinType type) {
  return std::ranges::any_of(variant_types, [&type](const VarType& param) {
    return param.type == type;
  });
}

bool Scope::identifier_in_variant(const std::vector<VarType>& variant_types, const std::string& identifier) const {
  if (!get_type(identifier)) {
    return false;
  }

  return std::ranges::any_of(variant_types, [&identifier](const VarType& param) {
    return param.type == IDENTIFIER && param.name == identifier;
  });
}

bool Scope::match_type(const eval_value_t& actual, const VarType& expected, bool check_self) const {
  if (check_self && !expected.name.empty() && expected.type == IDENTIFIER) {
    if (const auto& type = get_type(expected.name)) {
      if (const auto& variant = std::get_if<std::shared_ptr<VariantType>>(&*type)) {
        return std::visit(overloaded{
            [&](const value_t& value) {
              return std::visit(overloaded{
                  [&](std::monostate) { return type_in_variant(variant->get()->types, VOID); },
                  [&](const std::string &) { return type_in_variant(variant->get()->types, STR); },
                  [&](int) { return type_in_variant(variant->get()->types, INT); },
                  [&](float) { return type_in_variant(variant->get()->types, FLOAT); },
                  [&](bool) { return type_in_variant(variant->get()->types, BOOL); },
              }, value);
            },
            [&](const std::shared_ptr<Variable>& obj) { return identifier_in_variant(variant->get()->types, obj->type.name); },
            [&](const std::shared_ptr<StructObject>& obj) { return identifier_in_variant(variant->get()->types, obj->type_def->type_name); },
            [&](const std::shared_ptr<VariantObject>& obj) { return identifier_in_variant(variant->get()->types, obj->type_def->type_name) || obj->type_def->type_name == expected.name; },
            [](auto) { return false; }
        }, actual);
      }
    }
  }

  std::function<bool(const eval_value_t&)> check = [&](const auto& arg) -> bool {
    return std::visit(overloaded{
        [&expected](const value_t& v) {
          return std::visit(overloaded{
              [](auto) { return false; },
              [&expected](int) { return expected.type == INT; },
              [&expected](float) { return expected.type == FLOAT; },
              [&expected](const std::string&) { return expected.type == STR; },
              [&expected](bool) { return expected.type == BOOL; },
          }, v);
        },
        [&expected](const std::shared_ptr<Variable>& arg) { return arg->type.name == expected.name; },
        [&expected](const std::shared_ptr<StructObject>& arg) { return arg->type_def->type_name == expected.name; },
        [&](const std::shared_ptr<VariantObject>& arg) {
          return arg->type_def->type_name == expected.name || check(arg->contained);
        },
        [](auto) { return false; }
    }, arg);
  };

  return check(actual);
}

void Scope::define_variable(const std::string& name, eval_value_t variable) {
  variables.insert({name, std::move(variable)});
}

std::optional<eval_value_t> Scope::get_variable(const std::string& name) const {
  auto item = variables.find(name);
  if (item != variables.end()) {
    return item->second;
  }
  if (enclosing != nullptr) {
    return enclosing->get_variable(name);
  }
  return std::nullopt;
}

std::optional<types_t> Scope::get_type(const std::string &name) const {
  auto item = types.find(name);
  if (item != types.end()) {
    return item->second;
  }
  if (enclosing != nullptr) {
    return enclosing->get_type(name);
  }
  return std::nullopt;
}

std::optional<function_t> Scope::get_function(const std::string &name) const {
  auto item = functions.find(name);
  if (item != functions.end()) {
    return item->second;
  }
  if (enclosing != nullptr) {
    return enclosing->get_function(name);
  }
  return std::nullopt;
}

void Scope::define_type(const std::string &name, types_t type) {
  types.insert({name, std::move(type)});
}

void Scope::define_function(const std::string &name, function_t function) {
  functions.insert({name, std::move(function)});
}