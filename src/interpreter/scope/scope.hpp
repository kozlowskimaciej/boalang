/*! @file scope.hpp
    @brief boalang interpreter's scope.
*/

#ifndef BOALANG_SCOPE_HPP
#define BOALANG_SCOPE_HPP

#include <string>
#include <utility>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "stmt/stmt.hpp"
#include "utils/errors.hpp"
#include "token/token.hpp"

static constexpr unsigned int MAX_RECURSION_DEPTH = 50;

// forward declarations
struct Variable;
struct StructObject;
struct VariantObject;
struct InitalizerList;
struct FunctionObject;
class Scope;

using eval_value_t = std::variant<value_t, std::shared_ptr<StructObject>, std::shared_ptr<VariantObject>, std::shared_ptr<Variable>, std::shared_ptr<InitalizerList>>;
using function_t = std::shared_ptr<FunctionObject>;

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
  std::vector<Variable> init_fields;

  StructType(std::string type_name, std::vector<Variable> init_fields) : type_name(std::move(type_name)), init_fields(std::move(init_fields)) {};
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

struct InitalizerList {
  std::vector<eval_value_t> values;

  InitalizerList(std::vector<eval_value_t> values) : values(std::move(values)) {};
};

using types_t = std::variant<std::shared_ptr<StructType>, std::shared_ptr<VariantType>>;

struct FunctionObject {
  std::string identifier;
  VarType return_type;
  std::vector<std::pair<std::string, VarType>> params;
  BlockStmt* body;

  FunctionObject(std::string identifier, VarType return_type, std::vector<std::pair<std::string, VarType>> params, BlockStmt* body) :
      identifier(std::move(identifier)), return_type(std::move(return_type)), params(std::move(params)), body(body) {};
};

class Scope {
  std::map<std::string, eval_value_t> variables{};
  std::map<std::string, types_t> types{};
  std::map<std::string, function_t> functions{};
  Scope* enclosing;

 public:
  Scope() : enclosing(nullptr) {};
  Scope(Scope* enclosing) : enclosing(enclosing) {};

  void define_variable(const std::string& name, eval_value_t variable);
  void define_type(const std::string& name, types_t type);
  void define_function(const std::string& name, function_t function);
  [[nodiscard]] std::optional<eval_value_t> get_variable(const std::string& name) const;
  [[nodiscard]] std::optional<types_t> get_type(const std::string& name) const;
  [[nodiscard]] std::optional<function_t> get_function(const std::string& name) const;
  [[nodiscard]] bool match_type(const eval_value_t& actual, const VarType& expected, bool check_self = true) const;
  [[nodiscard]] static bool type_in_variant(const std::vector<VarType>& variant_types, BuiltinType type);
  [[nodiscard]] bool identifier_in_variant(const std::vector<VarType>& variant_types, const std::string& identifier) const;
};

struct StructObject {
  StructType* type_def;
  bool mut;
  std::string name;
  Scope scope;

  StructObject(StructType* type_def, bool mut, std::string name, Scope scope) :
      type_def(type_def), mut(mut), name(std::move(name)), scope(std::move(scope)) {};
};

class CallContext {
 public:
  function_t function;
  std::vector<std::unique_ptr<Scope>> scopes;

  CallContext(function_t function) : function(std::move(function)) {scopes.push_back(std::make_unique<Scope>());};
};

#endif  // BOALANG_SCOPE_HPP
