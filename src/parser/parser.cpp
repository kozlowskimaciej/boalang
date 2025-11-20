#include "parser.hpp"

#include <algorithm>

constexpr unsigned int MAX_ARGUMENTS =
    256; /**< Maximum number of function arguments supported by parser. */

const std::initializer_list<std::unique_ptr<Stmt> (Parser::*)()>
    Parser::stmt_handlers = {
        &Parser::if_stmt,     &Parser::while_stmt,   &Parser::return_stmt,
        &Parser::print_stmt,  &Parser::inspect_stmt, &Parser::block_stmt,
        &Parser::struct_decl, &Parser::variant_decl, &Parser::var_or_func,
};

const std::initializer_list<std::unique_ptr<Stmt> (Parser::*)()>
    Parser::declaration_handlers = {
        &Parser::mut_var_decl,
        &Parser::void_func_decl,
};

std::unique_ptr<Stmt> Parser::try_handlers(
    const Parser::StmtHandlers handlers) {
  for (const auto& handler : handlers) {
    if (auto stmt = ((*this).*handler)()) {
      return stmt;
    }
  }
  return nullptr;
}

// RULE program = { statement } ;
std::unique_ptr<Program> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (auto stmt = statement()) {
    statements.push_back(std::move(stmt));
  }
  if (!match(TOKEN_ETX)) {
    throw SyntaxError(current_token_, "Expected statement or declaration.");
  }
  return std::make_unique<Program>(std::move(statements), Position{0, 0});
}

// RULE statement = if_stmt
//                | while_stmt
//                | return_stmt
//                | print_stmt
//                | inspect_stmt
//                | block_stmt
//                |	struct_decl
//                | variant_decl
//                | var_or_func ;
std::unique_ptr<Stmt> Parser::statement() {
  return try_handlers(stmt_handlers);
}

// RULE if_stmt = "if" "(" expression ")" statement [ "else" statement ] ;
std::unique_ptr<Stmt> Parser::if_stmt() {
  if (auto token = match(TOKEN_IF)) {
    consume("Expected '(' after 'if'.", TOKEN_LPAREN);
    std::unique_ptr<Expr> condition = expression();
    if (!condition) {
      throw SyntaxError(current_token_, "Expected if condition statement.");
    }
    consume("Expected ')' after condition.", TOKEN_RPAREN);
    std::unique_ptr<Stmt> then_branch = statement();
    if (!then_branch) {
      throw SyntaxError(current_token_, "Expected if's then branch statement.");
    }
    std::unique_ptr<Stmt> else_branch;
    if (match(TOKEN_ELSE)) {
      else_branch = statement();
      if (!else_branch) {
        throw SyntaxError(current_token_,
                          "Expected if's else branch statement.");
      }
    }
    return std::make_unique<IfStmt>(
        std::move(condition), std::move(then_branch), std::move(else_branch),
        token->get_position());
  }
  return nullptr;
}

// RULE while_stmt = "while" "(" expression ")" statement ;
std::unique_ptr<Stmt> Parser::while_stmt() {
  if (auto token = match(TOKEN_WHILE)) {
    consume("Expected '(' after 'while'.", TOKEN_LPAREN);
    std::unique_ptr<Expr> condition = expression();
    if (!condition) {
      throw SyntaxError(current_token_, "Expected condition expression.");
    }
    consume("Expected ')' after while condition.", TOKEN_RPAREN);
    std::unique_ptr<Stmt> body = statement();
    if (!body) {
      throw SyntaxError(current_token_, "Expected body statement.");
    }

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body),
                                       token->get_position());
  }
  return nullptr;
}

// RULE return_stmt = "return" [ expression ] ";" ;
std::unique_ptr<Stmt> Parser::return_stmt() {
  if (auto token = match(TOKEN_RETURN)) {
    std::unique_ptr<Expr> value;
    if (!match(TOKEN_SEMICOLON)) {
      value = expression();
      if (!value) {
        throw SyntaxError(current_token_,
                          "Expected expression after 'return'.");
      }
      consume("Expected ';' after returned expression.", TOKEN_SEMICOLON);
    }
    return std::make_unique<ReturnStmt>(std::move(value),
                                        token->get_position());
  }
  return nullptr;
}

// RULE print_stmt = "print" expression ";" ;
std::unique_ptr<Stmt> Parser::print_stmt() {
  if (auto token = match(TOKEN_PRINT)) {
    std::unique_ptr<Expr> expr = expression();
    if (!expr) {
      throw SyntaxError(current_token_, "Expected expression after 'print'.");
    }
    consume("Expected ';' after printed expression.", TOKEN_SEMICOLON);
    return std::make_unique<PrintStmt>(std::move(expr), token->get_position());
  }
  return nullptr;
}

// RULE inspect_stmt = "inspect" expression "{" { lambda_func } [ "default" "=>"
// block_stmt ] "}" ;
std::unique_ptr<Stmt> Parser::inspect_stmt() {
  if (auto token = match(TOKEN_INSPECT)) {
    std::unique_ptr<Expr> inspected = expression();
    if (!inspected) {
      throw SyntaxError(current_token_, "Expected expression after 'inspect'.");
    }
    consume("Expected '{' after inspected expression.", TOKEN_LBRACE);
    std::vector<std::unique_ptr<LambdaFuncStmt>> lambdas;
    while (auto lambda = lambda_func()) {
      lambdas.push_back(std::move(lambda));
    }

    std::unique_ptr<Stmt> default_lambda;
    if (match(TOKEN_DEFAULT)) {
      consume("Expected '=>' after default lambda.", TOKEN_ARROW);
      default_lambda = block_stmt();
      if (!default_lambda) {
        throw SyntaxError(current_token_,
                          "Expected block statement after '=>'.");
      }
    }
    consume("Expected '}' after inspect lambdas.", TOKEN_RBRACE);
    return std::make_unique<InspectStmt>(
        std::move(inspected), std::move(lambdas), token->get_position(),
        std::move(default_lambda));
  }
  return nullptr;
}

// RULE lambda_func = type identifier "=>" block_stmt
std::unique_ptr<LambdaFuncStmt> Parser::lambda_func() {
  std::optional<Token> lambda_type = type();
  if (!lambda_type) {
    return nullptr;
  }
  Token lambda_id =
      consume("Expected identifier after lambda type.", TOKEN_IDENTIFIER);
  consume("Expected '=>' after lambda identifier.", TOKEN_ARROW);
  std::unique_ptr<Stmt> lambda_body = block_stmt();
  if (!lambda_body) {
    throw SyntaxError(current_token_, "Expected statement for lambda body.");
  }
  return std::make_unique<LambdaFuncStmt>(
      lambda_type->get_var_type(), lambda_id.stringify(),
      std::move(lambda_body), lambda_id.get_position());
}

// RULE block_stmt = "{" { statement } "}" ;
std::unique_ptr<Stmt> Parser::block_stmt() {
  if (auto token = match(TOKEN_LBRACE)) {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (auto stmt = statement()) {
      statements.push_back(std::move(stmt));
    }
    consume("Expected '}' after block statement.", TOKEN_RBRACE);
    return std::make_unique<BlockStmt>(std::move(statements),
                                       token->get_position());
  }
  return nullptr;
}

// RULE struct_decl = "struct" identifier "{" { struct_field } "}" ;
std::unique_ptr<Stmt> Parser::struct_decl() {
  if (!match(TOKEN_STRUCT)) {
    return nullptr;
  }
  Token struct_id =
      consume("Expected identifier after 'struct'.", TOKEN_IDENTIFIER);
  consume("Expected '{' after struct identifier.", TOKEN_LBRACE);
  std::vector<std::unique_ptr<StructFieldStmt>> fields;
  while (auto field = struct_field()) {
    fields.push_back(std::move(field));
  }
  consume("Expected '}' after struct declaration.", TOKEN_RBRACE);
  return std::make_unique<StructDeclStmt>(
      struct_id.stringify(), std::move(fields), struct_id.get_position());
}

// RULE struct_field = [ "mut" ] type identifier ";" ;
std::unique_ptr<StructFieldStmt> Parser::struct_field() {
  bool is_mut = false;
  if (match(TOKEN_MUT)) {
    is_mut = true;
  }
  std::optional<Token> field_type = type();
  if (!field_type) {
    if (is_mut) {
      throw SyntaxError(current_token_, "Expected struct field type.");
    }
    return nullptr;
  }
  Token field_id =
      consume("Expected identifier after struct field type.", TOKEN_IDENTIFIER);
  consume("Expected ';' after struct field.", TOKEN_SEMICOLON);
  return std::make_unique<StructFieldStmt>(field_type->get_var_type(),
                                           field_id.stringify(),
                                           field_id.get_position(), is_mut);
}

// RULE variant_decl = "variant" identifier "{" variant_params "}" ";" ;
std::unique_ptr<Stmt> Parser::variant_decl() {
  if (!match(TOKEN_VARIANT)) {
    return nullptr;
  }
  Token variant_id = consume("Expected variant identifier.", TOKEN_IDENTIFIER);
  consume("Expected '{' after identifier.", TOKEN_LBRACE);
  std::vector<VarType> params;
  if (auto variantparams = variant_params()) {
    params = *variantparams;
  } else {
    throw SyntaxError(current_token_, "Expected variant parameters.");
  }
  consume("Expected '}' after variant parameters.", TOKEN_RBRACE);
  consume("Expected ';' after variant declaration.", TOKEN_SEMICOLON);
  return std::make_unique<VariantDeclStmt>(
      variant_id.stringify(), std::move(params), variant_id.get_position());
}

// RULE variant_params = type { "," type } ;
std::optional<std::vector<VarType>> Parser::variant_params() {
  std::vector<VarType> params;

  if (auto type_token = type()) {
    params.push_back(type_token->get_var_type());
  } else {
    return std::nullopt;
  }

  while (match(TOKEN_COMMA)) {
    auto type_token = type();
    if (!type_token) {
      throw SyntaxError(current_token_, "Expected variant parameter type.");
    }
    params.push_back(type_token->get_var_type());
  }
  return params;
}

// RULE var_or_func = mut_var_decl
//                  | void_func_decl
//                  | identifier assign_or_call
//                  | type var_or_func_decl ;
std::unique_ptr<Stmt> Parser::var_or_func() {
  if (auto stmt = try_handlers(declaration_handlers)) {
    return stmt;
  }

  if (auto token = match(TOKEN_IDENTIFIER)) {
    if (auto assigncall = assign_or_call(*token)) {
      return assigncall;
    }
    if (auto varfuncdecl = var_or_func_decl(*token)) {
      return varfuncdecl;
    }
    throw SyntaxError(current_token_,
                      "Expected assignment, call or declaration.");
  }

  if (auto decl_type = type()) {
    if (auto varfuncdecl = var_or_func_decl(*decl_type)) {
      return varfuncdecl;
    }
    throw SyntaxError(current_token_,
                      "Expected variable or function declaration.");
  }

  return nullptr;
}

// RULE assign_or_call = ( assign_stmt | call_stmt ) ;
std::unique_ptr<Stmt> Parser::assign_or_call(const Token& identifier) {
  if (auto call = call_stmt(identifier)) {
    return call;
  }
  auto id_expr = std::make_unique<VarExpr>(identifier.stringify(),
                                           identifier.get_position());
  if (auto assign = assign_stmt(std::move(id_expr))) {
    return assign;
  }
  return nullptr;
}

// RULE assign = [ "." field_access ] "=" expression ";" ;
std::unique_ptr<AssignStmt> Parser::assign_stmt(std::unique_ptr<Expr> var) {
  bool is_field = false;

  if (match(TOKEN_DOT)) {
    var = field_access(std::move(var));
    is_field = true;
  }

  if (auto token = match(TOKEN_EQUAL)) {
    auto value = expression();
    if (!value) {
      throw SyntaxError(current_token_, "Expected expression for assignment.");
    }
    consume("Expected ';' after assignment.", TOKEN_SEMICOLON);
    return std::make_unique<AssignStmt>(std::move(var), std::move(value),
                                        token->get_position());
  }
  if (is_field) {
    throw SyntaxError(current_token_,
                      "Expected '=' after field access for assignment.");
  }
  return nullptr;
}

// RULE call_stmt = "(" [ arguments ] ");" ;
std::unique_ptr<CallStmt> Parser::call_stmt(const Token& identifier) {
  if (!match(TOKEN_LPAREN)) {
    return nullptr;
  }
  if (match(TOKEN_RPAREN)) {
    consume("Expected ';' after call statement.", TOKEN_SEMICOLON);
    return std::make_unique<CallStmt>(identifier.stringify(),
                                      identifier.get_position());
  }

  std::vector<std::unique_ptr<Expr>> call_args;
  if (!match(TOKEN_RPAREN)) {
    if (auto args = arguments()) {
      call_args = std::move(*args);
    } else {
      throw SyntaxError(current_token_, "Expected call arguments.");
    }
    consume("Excepted ')' after call arguments.", TOKEN_RPAREN);
  }
  consume("Expected ';' after call statement.", TOKEN_SEMICOLON);
  return std::make_unique<CallStmt>(
      identifier.stringify(), identifier.get_position(), std::move(call_args));
}

// RULE var_or_func_decl = identifier ( var_decl | func_decl ) ;
std::unique_ptr<Stmt> Parser::var_or_func_decl(const Token& type) {
  if (auto identifier = match(TOKEN_IDENTIFIER)) {
    if (auto vardecl = var_decl(type, *identifier, false)) {
      return vardecl;
    }
    if (auto funcdecl = func_decl(type, *identifier)) {
      return funcdecl;
    }
    throw SyntaxError(current_token_,
                      "Expected variable or function declaration.");
  }
  return nullptr;
}

// RULE mut_var_decl = "mut" type identifier var_decl ;
std::unique_ptr<Stmt> Parser::mut_var_decl() {
  if (!match(TOKEN_MUT)) {
    return nullptr;
  }
  auto var_type = type();
  if (!var_type) {
    throw SyntaxError(current_token_, "Expected variable type.");
  }
  Token identifier =
      consume("Expected identifier after variable type.", TOKEN_IDENTIFIER);
  if (auto vardecl = var_decl(*var_type, identifier, true)) {
    return vardecl;
  }
  throw SyntaxError(current_token_, "Expected variable declaration.");
}

// RULE void_func_decl = "void" identifier func_decl ;
std::unique_ptr<Stmt> Parser::void_func_decl() {
  if (auto return_type = match(TOKEN_VOID)) {
    Token identifier = consume(
        "Expected identifier after function return type.", TOKEN_IDENTIFIER);
    if (auto funcdecl = func_decl(*return_type, identifier)) {
      return funcdecl;
    }
    throw SyntaxError(current_token_, "Expected function declaration.");
  }
  return nullptr;
}

// RULE var_decl = "=" expression ";" ;
std::unique_ptr<VarDeclStmt> Parser::var_decl(const Token& type,
                                              const Token& identifier,
                                              bool mut) {
  if (!match(TOKEN_EQUAL)) {
    return nullptr;
  }
  std::unique_ptr<Expr> expr = expression();
  if (!expr) {
    throw SyntaxError(current_token_, "Expected expression.");
  }
  consume("Expected ';' after variable declaration.", TOKEN_SEMICOLON);
  return std::make_unique<VarDeclStmt>(type.get_var_type(),
                                       identifier.stringify(), std::move(expr),
                                       identifier.get_position(), mut);
}

// RULE func_decl = "(" [ func_params ] ")" block_stmt ;
std::unique_ptr<FuncStmt> Parser::func_decl(const Token& return_type,
                                            const Token& identifier) {
  if (!match(TOKEN_LPAREN)) {
    return nullptr;
  }
  std::vector<std::unique_ptr<FuncParamStmt>> params;
  if (!match(TOKEN_RPAREN)) {
    if (auto funcparams = func_params()) {
      params = std::move(*funcparams);
    } else {
      throw SyntaxError(current_token_, "Expected function parameters.");
    }
    consume("Excepted ')' after function parameters.", TOKEN_RPAREN);
  }
  auto body = block_stmt();
  if (!body) {
    throw SyntaxError(current_token_,
                      "Expected block statement in function declaration.");
  }
  return std::make_unique<FuncStmt>(
      identifier.stringify(), return_type.get_var_type(), std::move(params),
      std::move(body), identifier.get_position());
}

// RULE func_params = type identifier { "," type identifier } ;
std::optional<std::vector<std::unique_ptr<FuncParamStmt>>>
Parser::func_params() {
  std::vector<std::unique_ptr<FuncParamStmt>> params;

  do {
    std::optional<Token> param_type = type();
    if (!param_type) {
      if (params.empty()) {
        return std::nullopt;
      }
      throw SyntaxError(current_token_, "Expected function parameter type.");
    }
    Token param_id =
        consume("Expected identifier after type.", TOKEN_IDENTIFIER);
    params.push_back(std::make_unique<FuncParamStmt>(param_type->get_var_type(),
                                                     param_id.stringify(),
                                                     param_id.get_position()));
  } while (match(TOKEN_COMMA));

  return params;
}

// RULE expression = logic_or ;
std::unique_ptr<Expr> Parser::expression() { return logic_or(); }

// RULE logic_or = logic_and { "or" logic_and } ;
std::unique_ptr<Expr> Parser::logic_or() {
  std::unique_ptr<Expr> expr = logic_and();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_OR)) {
    std::unique_ptr<Expr> right = logic_and();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    expr = std::make_unique<LogicalOrExpr>(std::move(expr), std::move(right),
                                           token->get_position());
  }

  return expr;
}

// RULE logic_and = equality { "and" equality } ;
std::unique_ptr<Expr> Parser::logic_and() {
  std::unique_ptr<Expr> expr = equality();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_AND)) {
    std::unique_ptr<Expr> right = equality();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    expr = std::make_unique<LogicalAndExpr>(std::move(expr), std::move(right),
                                            token->get_position());
  }

  return expr;
}

// RULE equality = comparison { ( "!=" | "==" ) comparison } ;
std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_NOT_EQUAL, TOKEN_EQUAL_EQUAL)) {
    std::unique_ptr<Expr> right = comparison();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    switch (token->get_type()) {
      case TOKEN_NOT_EQUAL:
        expr = std::make_unique<NotEqualCompExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      case TOKEN_EQUAL_EQUAL:
        expr = std::make_unique<EqualCompExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      default:
        break;
    }
  }

  return expr;
}

// RULE comparison = term { ( ">" | ">=" | "<" | "<=" ) term } ;
std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS,
                            TOKEN_LESS_EQUAL)) {
    std::unique_ptr<Expr> right = term();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    switch (token->get_type()) {
      case TOKEN_GREATER:
        expr = std::make_unique<GreaterCompExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      case TOKEN_GREATER_EQUAL:
        expr = std::make_unique<GreaterEqualCompExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      case TOKEN_LESS:
        expr = std::make_unique<LessCompExpr>(std::move(expr), std::move(right),
                                              token->get_position());
        break;
      case TOKEN_LESS_EQUAL:
        expr = std::make_unique<LessEqualCompExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      default:
        break;
    }
  }

  return expr;
}

// RULE term = factor { ( "-" | "+" ) factor } ;
std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_MINUS, TOKEN_PLUS)) {
    std::unique_ptr<Expr> right = factor();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    switch (token->get_type()) {
      case TOKEN_MINUS:
        expr = std::make_unique<SubtractionExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      case TOKEN_PLUS:
        expr = std::make_unique<AdditionExpr>(std::move(expr), std::move(right),
                                              token->get_position());
        break;
      default:
        break;
    }
  }

  return expr;
}

// RULE factor = unary { ( "/" | "*" ) unary } ;
std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_SLASH, TOKEN_STAR)) {
    std::unique_ptr<Expr> right = unary();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    switch (token->get_type()) {
      case TOKEN_SLASH:
        expr = std::make_unique<DivisionExpr>(std::move(expr), std::move(right),
                                              token->get_position());
        break;
      case TOKEN_STAR:
        expr = std::make_unique<MultiplicationExpr>(
            std::move(expr), std::move(right), token->get_position());
        break;
      default:
        break;
    }
  }

  return expr;
}

// RULE unary = [ "!" | "-" ] type_cast ;
std::unique_ptr<Expr> Parser::unary() {
  if (auto token = match(TOKEN_EXCLAMATION, TOKEN_MINUS)) {
    std::unique_ptr<Expr> right = type_cast();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    switch (token->get_type()) {
      case TOKEN_EXCLAMATION:
        return std::make_unique<LogicalNegationExpr>(std::move(right),
                                                     token->get_position());
      case TOKEN_MINUS:
        return std::make_unique<NegationExpr>(std::move(right),
                                              token->get_position());
      default:
        break;
    }
  }

  return type_cast();
}

// RULE type_cast = call { ("as" | "is") type } ;
std::unique_ptr<Expr> Parser::type_cast() {
  std::unique_ptr<Expr> expr = call();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match(TOKEN_AS, TOKEN_IS)) {
    auto cast_type = type();
    if (!cast_type) {
      throw SyntaxError(current_token_, "Expected cast type.");
    }
    switch (token->get_type()) {
      case TOKEN_AS:
        expr = std::make_unique<AsTypeExpr>(
            std::move(expr), cast_type->get_var_type(), token->get_position());
        break;
      case TOKEN_IS:
        expr = std::make_unique<IsTypeExpr>(
            std::move(expr), cast_type->get_var_type(), token->get_position());
        break;
      default:
        break;
    }
  }

  return expr;
}

// RULE call = primary { "(" [ arguments ] ")" | field_access };
std::unique_ptr<Expr> Parser::call() {
  std::unique_ptr<Expr> expr = primary();

  if (!expr) {
    return nullptr;
  }

  if (match(TOKEN_LPAREN)) {
    std::vector<std::unique_ptr<Expr>> call_args;
    if (!match(TOKEN_RPAREN)) {
      auto args = arguments();
      if (!args) {
        throw SyntaxError(current_token_, "Expected arguments in call.");
      }
      call_args = std::move(*args);
      consume("Excepted ')' after call arguments.", TOKEN_RPAREN);
    }
    auto* var = dynamic_cast<VarExpr*>(expr.get());
    if (!var) {
      throw SyntaxError(current_token_, "Expected identifier as callee.");
    }
    expr = std::make_unique<CallExpr>(var->identifier, var->position,
                                      std::move(call_args));
  } else if (match(TOKEN_DOT)) {
    expr = field_access(std::move(expr));
  }

  return expr;
}

// RULE primary = string | int_val | float_val | bool_values | identifier | "("
// expression ")" | "{" arguments "}" ;
std::unique_ptr<Expr> Parser::primary() {
  if (auto token = match(TOKEN_FLOAT_VAL, TOKEN_INT_VAL, TOKEN_STR_VAL,
                         TOKEN_TRUE, TOKEN_FALSE)) {
    return std::make_unique<LiteralExpr>(token->get_value(),
                                         token->get_position());
  }

  if (auto token = match(TOKEN_IDENTIFIER)) {
    return std::make_unique<VarExpr>(token->stringify(), token->get_position());
  }

  if (auto token = match(TOKEN_LPAREN)) {
    std::unique_ptr<Expr> expr = expression();
    if (!expr) {
      throw SyntaxError(current_token_, "Expected expression after '('.");
    }
    consume("Excepted ')' after expression.", TOKEN_RPAREN);
    return std::make_unique<GroupingExpr>(std::move(expr),
                                          token->get_position());
  }

  if (auto token = match(TOKEN_LBRACE)) {
    auto args = arguments();
    if (!args) {
      throw SyntaxError(current_token_,
                        "Expected arguments for initalizer list.");
    }
    consume("Excepted '}' after initializer list.", TOKEN_RBRACE);
    return std::make_unique<InitalizerListExpr>(std::move(*args),
                                                token->get_position());
  }

  return nullptr;
}

// RULE arguments = expression { "," expression } ;
std::optional<std::vector<std::unique_ptr<Expr>>> Parser::arguments() {
  std::vector<std::unique_ptr<Expr>> args;

  if (auto expr = expression()) {
    args.push_back(std::move(expr));
  } else {
    return std::nullopt;
  }

  if (match(TOKEN_COMMA)) {
    while (auto expr = expression()) {
      if (args.size() > MAX_ARGUMENTS) {
        throw SyntaxError(current_token_, "Maximum amount (" +
                                              std::to_string(MAX_ARGUMENTS) +
                                              ") of arguments exceeded.");
      }
      args.push_back(std::move(expr));
      if (!match(TOKEN_COMMA)) {
        break;
      }
    }
  }

  return args;
}

// RULE field_access = { "." identifier } ;
std::unique_ptr<Expr> Parser::field_access(
    std::unique_ptr<Expr> parent_struct) {
  do {
    auto id = consume("Expected identifier after '.' for accessing field.",
                      TOKEN_IDENTIFIER);
    parent_struct = std::make_unique<FieldAccessExpr>(
        std::move(parent_struct), id.stringify(), id.get_position());
  } while (match(TOKEN_DOT));
  return parent_struct;
}

// RULE type = "bool" | "str" | "int" | "float" | identifier ;
std::optional<Token> Parser::type() {
  if (auto token = match(TOKEN_FLOAT, TOKEN_INT, TOKEN_STR, TOKEN_BOOL,
                         TOKEN_IDENTIFIER)) {
    return *token;
  }

  return std::nullopt;
}

template <typename... TokenTypes>
opt_token_t Parser::match(TokenTypes&&... types) {
  if (((current_token_.get_type() == types) || ...)) {
    auto prev = current_token_;
    advance();
    return prev;
  }
  return std::nullopt;
}

Token Parser::advance() { return current_token_ = lexer_.next_token(); }

template <typename... TokenTypes>
std::enable_if_t<(std::is_same_v<TokenTypes, TokenType> && ...), Token>
Parser::consume(const std::string& err_msg, TokenTypes... types) {
  if (auto token = match(types...)) {
    return *token;
  }
  throw SyntaxError(current_token_, err_msg);
}
