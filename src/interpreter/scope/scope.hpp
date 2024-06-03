/*! @file scope.hpp
    @brief boalang interpreter's scope.
*/

#ifndef BOALANG_SCOPE_HPP
#define BOALANG_SCOPE_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "stmt/stmt.hpp"
#include "token/token.hpp"
#include "utils/errors.hpp"
#include "utils/overloaded.tpp"

static constexpr unsigned int MAX_RECURSION_DEPTH =
    50; /**< Maximum supported recursion depth. */

// forward declarations
struct Variable;
struct StructObject;
struct VariantObject;
struct InitalizerList;
struct FunctionObject;
struct StructType;
struct VariantType;

using eval_value_t =
    std::variant<value_t, std::shared_ptr<StructObject>,
                 std::shared_ptr<VariantObject>, std::shared_ptr<Variable>,
                 std::shared_ptr<InitalizerList>>; /**< Possible values returned
                                                      from evaluation. */

using function_t = std::shared_ptr<FunctionObject>; /**< Callable objects. */

using types_t =
    std::variant<std::shared_ptr<StructType>,
                 std::shared_ptr<VariantType>>; /**< Types declared in code. */

/**
 * @brief Scope representation.
 */
class Scope {
  std::map<std::string, eval_value_t>
      variables{};                        /**< Variables defined in scope. */
  std::map<std::string, types_t> types{}; /**< Types defined in scope. */
  std::map<std::string, function_t>
      functions{};  /**< Functions defined in scope. */
  Scope* enclosing; /**< Parent Scope. */

  [[nodiscard]] bool is_in_variant(const eval_value_t& actual,
                                const VarType& expected,
                                bool check_self = true) const;

 public:
  /**
   * @brief Constructs a new Scope object with no enclosing Scope.
   */
  Scope() : enclosing(nullptr){};

  /**
   * @brief Constructs a new Scope object with enclosing Scope.
   */
  Scope(Scope* enclosing) : enclosing(enclosing){};

  /**
   * @brief Defines new variable in current scope.
   */
  void define_variable(const std::string& name, eval_value_t variable);

  /**
   * @brief Defines new type in current scope.
   */
  void define_type(const std::string& name, types_t type);

  /**
   * @brief Defines new function in current scope.
   */
  void define_function(const std::string& name, function_t function);

  /**
   * @brief Gets variable from current scope.
   */
  [[nodiscard]] std::optional<eval_value_t> get_variable(
      const std::string& name) const;

  /**
   * @brief Gets type from current scope.
   */
  [[nodiscard]] std::optional<types_t> get_type(const std::string& name) const;

  /**
   * @brief Gets function from current scope.
   */
  [[nodiscard]] std::optional<function_t> get_function(
      const std::string& name) const;

  /**
   * @brief Match type of eval_value_t actual against types declared in current
   * scope.
   */
  [[nodiscard]] bool match_type(const eval_value_t& actual,
                                const VarType& expected,
                                bool check_self = true) const;

  /**
   * @brief Checks whether BuiltinType type is contained in variant_types.
   */
  [[nodiscard]] static bool type_in_variant(
      const std::vector<VarType>& variant_types, BuiltinType type);

  /**
   * @brief Checks whether identifier is contained in variant_types.
   */
  [[nodiscard]] bool identifier_in_variant(
      const std::vector<VarType>& variant_types,
      const std::string& identifier) const;
};

/**
 * @brief Variable representation.
 */
struct Variable {
  VarType type;
  std::string name;
  bool mut; /**< Is mutable. */
  std::optional<eval_value_t> value;

  /**
   * @brief Constructs a new Variable with eval_value_t value.
   */
  Variable(VarType type, std::string name, bool mut, eval_value_t value)
      : type(std::move(type)),
        name(std::move(name)),
        mut(mut),
        value(std::move(value)){};
  /**
   * @brief Constructs a new Variable with no value.
   */
  Variable(VarType type, std::string name, bool mut)
      : type(std::move(type)), name(std::move(name)), mut(mut){};
};

/**
 * @brief Struct type representation.
 */
struct StructType {
  std::string type_name;
  std::vector<Variable> init_fields; /**< Fields defined in struct. */

  StructType(std::string type_name, std::vector<Variable> init_fields)
      : type_name(std::move(type_name)), init_fields(std::move(init_fields)){};
};

/**
 * @brief Variant type representation.
 */
struct VariantType {
  std::string type_name;
  std::vector<VarType> types; /**< Types defined in variant. */

  VariantType(std::string type_name, std::vector<VarType> types)
      : type_name(std::move(type_name)), types(std::move(types)){};
};

/**
 * @brief Variant object representation.
 */
struct VariantObject {
  VariantType* type_def; /**< Pointer to type definition. */
  bool mut;              /**< Is mutable. */
  std::string name;
  eval_value_t contained; /**< Value currently contained in variant object. */

  VariantObject(VariantType* type_def, bool mut, std::string name,
                eval_value_t contained)
      : type_def(type_def),
        mut(mut),
        name(std::move(name)),
        contained(std::move(contained)){};
};

/**
 * @brief Initalizer list object representation.
 */
struct InitalizerList {
  std::vector<eval_value_t> values; /**< Initalizer list items. */

  InitalizerList(std::vector<eval_value_t> values)
      : values(std::move(values)){};
};

/**
 * @brief Struct object representation.
 */
struct StructObject {
  StructType* type_def; /**< Pointer to type definition. */
  bool mut;             /**< Is mutable. */
  std::string name;
  Scope scope; /**< Scope containing fields (variables). */

  StructObject(StructType* type_def, bool mut, std::string name, Scope scope)
      : type_def(type_def),
        mut(mut),
        name(std::move(name)),
        scope(std::move(scope)){};
};

/**
 * @brief Function object representation.
 */
struct FunctionObject {
  std::string identifier;
  VarType return_type;
  std::vector<std::pair<std::string, VarType>>
      params;      /**< Function's parameters. */
  BlockStmt* body; /**< Pointer to function's body. */

  FunctionObject(std::string identifier, VarType return_type,
                 std::vector<std::pair<std::string, VarType>> params,
                 BlockStmt* body)
      : identifier(std::move(identifier)),
        return_type(std::move(return_type)),
        params(std::move(params)),
        body(body){};
};

/**
 * @brief Call context representation.
 */
class CallContext {
 public:
  function_t function; /**< Pointer to function. */
  std::vector<std::unique_ptr<Scope>>
      scopes; /**< Vector of scopes limited to current call context. */

  CallContext(function_t function) : function(std::move(function)) {
    scopes.push_back(std::make_unique<Scope>());
  };
};

#endif  // BOALANG_SCOPE_HPP
