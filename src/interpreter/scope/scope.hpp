/*! @file scope.hpp
    @brief boalang interpreter's scope.
*/

#ifndef BOALANG_SCOPE_HPP
#define BOALANG_SCOPE_HPP

#include <string>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "utils/errors.hpp"
#include "token/token.hpp"

// forward declarations
struct Variable;
struct StructObject;
struct VariantObject;
//struct FunctionObject;
class Scope;

using eval_value_t = std::variant<value_t, std::shared_ptr<StructObject>, std::shared_ptr<VariantObject>, std::shared_ptr<Variable>>;  // , FunctionObject
using var_t = std::variant<std::shared_ptr<Variable>, std::shared_ptr<StructObject>, std::shared_ptr<VariantObject>>;

struct Variable {
  VarType type;
  std::string name;
  bool mut;
  std::optional<eval_value_t> value;

  Variable(VarType type, std::string name, bool mut, eval_value_t value)
      : type(std::move(type)), name(std::move(name)), mut(mut), value(std::move(value)) {}
  Variable(VarType type, std::string name, bool mut)
      : type(std::move(type)), name(std::move(name)), mut(mut), value(std::nullopt) {}
};

struct StructType {
  std::string type_name;
  std::map<std::string, Variable> init_fields;

  StructType(std::string type_name, std::map<std::string, Variable> init_fields) : type_name(std::move(type_name)), init_fields(std::move(init_fields)) {};
};

struct StructObject {
  StructType* type_def;
  std::string name;
  std::unique_ptr<Scope> scope;

  StructObject(StructType* type_def, std::string name, std::unique_ptr<Scope> scope) :
      type_def(type_def), name(std::move(name)), scope(std::move(scope)) {};
};

struct VariantType {
  std::string type_name;
  std::vector<VarType> types;

  VariantType(std::string type_name, std::vector<VarType> types) : type_name(std::move(type_name)), types(std::move(types)) {};
};

struct VariantObject {
  VariantType* type_def;
  bool mut;
  std::string name;
  eval_value_t contained;

  VariantObject(VariantType* type_def, bool mut, std::string name, eval_value_t contained) :
      type_def(type_def), mut(mut), name(std::move(name)), contained(std::move(contained)) {};
};

//struct FunctionObject {
//
//};

using types_t = std::variant<std::shared_ptr<StructType>, std::shared_ptr<VariantType>>;

class Scope {
  std::map<std::string, var_t> variables{};
  std::map<std::string, types_t> types{};
  Scope* enclosing;

 public:
  Scope() : enclosing(nullptr) {};
  Scope(Scope* enclosing) : enclosing(enclosing) {};

  void define(const std::string& name, var_t variable);
  void define_type(const std::string& name, types_t type);
  void assign(const std::string& name, eval_value_t new_value);
  [[nodiscard]] std::optional<var_t> get(const std::string& name) const;
  [[nodiscard]] std::optional<types_t> get_type(const std::string& name) const;
  [[nodiscard]] bool match_type(const eval_value_t& actual, const VarType& expected, bool check_self = true) const;
  [[nodiscard]] bool type_in_variant(const std::vector<VarType>& variant_types, BuiltinType type) const;
  [[nodiscard]] bool identifier_in_variant(const std::vector<VarType>& variant_types, const std::string& identifier) const;
};

#endif  // BOALANG_SCOPE_HPP
