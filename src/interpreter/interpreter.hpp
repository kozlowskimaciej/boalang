/*! @file interpreter.hpp
    @brief boalang interpreter.
*/

#ifndef BOALANG_INTERPRETER_HPP
#define BOALANG_INTERPRETER_HPP

#include <optional>
#include <vector>

#include "expr/expr.hpp"
#include "stmt/stmt.hpp"
#include "interpreter/scope/scope.hpp"
#include "utils/errors.hpp"

class Interpreter : public ExprVisitor, public StmtVisitor {
  std::optional<eval_value_t> evaluation = std::nullopt;

  template <typename VisitType>
  eval_value_t evaluate(const VisitType* visited);

  template <typename VisitType>
  eval_value_t evaluate_var(const VisitType* visited);

  void set_evaluation(eval_value_t value);
  eval_value_t get_evaluation();

  static bool boolify(const eval_value_t& value);
  std::vector<std::unique_ptr<Scope>> scopes;
  std::vector<std::unique_ptr<CallContext>> call_contexts;

  bool return_flag = false;

  Scope* create_new_scope();
  void pop_last_scope();

  void call_func(FunctionObject* func);
  std::vector<eval_value_t> get_call_args_values(const std::vector<std::unique_ptr<Expr>>& arguments);
  void create_call_context(const std::shared_ptr<FunctionObject>& func, const Position& position);
  void pop_call_context();
  void bind_args_to_params(const FunctionObject* func, const std::vector<eval_value_t>& args, const Position& position);
  void make_call(const std::string& identifier, const Position& position, const std::vector<std::unique_ptr<Expr>>& arguments);
  void define_variable(const std::string& name, const eval_value_t &variable);
  void define_type(const std::string& name, const types_t &type);
  void define_function(const std::string& name, const function_t &function);
  [[nodiscard]] std::optional<eval_value_t> get_variable(const std::string& name) const;
  [[nodiscard]] std::optional<types_t> get_type(const std::string& name) const;
  [[nodiscard]] std::optional<function_t> get_function(const std::string& name) const;
  [[nodiscard]] bool match_type(const eval_value_t& actual, const VarType& expected, bool check_self = true) const;

  template<typename Operation>
  void perform_arithmetic_operation(Expr* left, Expr* right, Operation op, const Position& position);

  template<typename Operation>
  void perform_comparison_operation(Expr* left, Expr* right, Operation op, const Position& position);
 public:
  Interpreter() { scopes.push_back(std::make_unique<Scope>()); };
  void visit(const Program& stmt) override;
  void visit(const PrintStmt& stmt) override;
  void visit(const IfStmt& stmt) override;
  void visit(const BlockStmt& stmt) override;
  void visit(const WhileStmt& stmt) override;
  void visit(const VarDeclStmt& stmt) override;
  void visit(const StructFieldStmt& stmt) override;
  void visit(const StructDeclStmt& stmt) override;
  void visit(const VariantDeclStmt& stmt) override;
  void visit(const AssignStmt& stmt) override;
  void visit(const CallStmt& stmt) override;
  void visit(const FuncParamStmt& stmt) override;
  void visit(const FuncStmt& stmt) override;
  void visit(const ReturnStmt& stmt) override;
  void visit(const LambdaFuncStmt& stmt) override;
  void visit(const InspectStmt& stmt) override;

  void visit(const AdditionExpr& expr) override;
  void visit(const SubtractionExpr& expr) override;
  void visit(const DivisionExpr& expr) override;
  void visit(const MultiplicationExpr& expr) override;
  void visit(const EqualCompExpr& expr) override;
  void visit(const NotEqualCompExpr& expr) override;
  void visit(const GreaterCompExpr& expr) override;
  void visit(const GreaterEqualCompExpr& expr) override;
  void visit(const LessCompExpr& expr) override;
  void visit(const LessEqualCompExpr& expr) override;
  void visit(const GroupingExpr& expr) override;
  void visit(const LiteralExpr& expr) override;
  void visit(const NegationExpr& expr) override;
  void visit(const LogicalNegationExpr& expr) override;
  void visit(const VarExpr& expr) override;
  void visit(const LogicalOrExpr& expr) override;
  void visit(const LogicalAndExpr& expr) override;
  void visit(const IsTypeExpr& expr) override;
  void visit(const AsTypeExpr& expr) override;
  void visit(const InitalizerListExpr& expr) override;
  void visit(const CallExpr& expr) override;
  void visit(const FieldAccessExpr& expr) override;
};

#endif  // BOALANG_INTERPRETER_HPP
