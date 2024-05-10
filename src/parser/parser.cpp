#include "parser.hpp"

#include <algorithm>

constexpr unsigned int MAX_ARGUMENTS =
    256; /**< Maximum number of function arguments supported by parser. */

// RULE program = { statement } ;
std::unique_ptr<Program> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (auto stmt = statement()) {
    statements.push_back(std::move(stmt));
  }
  if (!match({TOKEN_ETX})) {
    throw SyntaxError(current_token_, "Expected statement or declaration.");
  }
  return std::make_unique<Program>(std::move(statements));
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
  std::vector<std::function<std::unique_ptr<Stmt>()>> stmt_handlers = {
      [this]() { return if_stmt(); },      [this]() { return while_stmt(); },
      [this]() { return return_stmt(); },  [this]() { return print_stmt(); },
      [this]() { return inspect_stmt(); }, [this]() { return block_stmt(); },
      [this]() { return struct_decl(); },  [this]() { return variant_decl(); },
      [this]() { return var_or_func(); }};

  for (auto& handler : stmt_handlers) {
    if (auto stmt = handler()) {
      return stmt;
    }
  }

  return nullptr;
}

// RULE if_stmt = "if" "(" expression ")" statement [ "else" statement ] ;
std::unique_ptr<IfStmt> Parser::if_stmt() {
  if (!match({TOKEN_IF})) {
    return nullptr;
  }
  consume({TOKEN_LPAREN}, "Expected '(' after 'if'.");
  std::unique_ptr<Expr> condition = expression();
  if (!condition) {
    throw SyntaxError(current_token_, "Expected if condition statement.");
  }
  consume({TOKEN_RPAREN}, "Expected ')' after condition.");
  std::unique_ptr<Stmt> then_branch = statement();
  if (!then_branch) {
    throw SyntaxError(current_token_, "Expected if's then branch statement.");
  }
  std::unique_ptr<Stmt> else_branch;
  if (match({TOKEN_ELSE})) {
    else_branch = statement();
    if (!else_branch) {
      throw SyntaxError(current_token_, "Expected if's else branch statement.");
    }
  }
  return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch),
                                  std::move(else_branch));
}

// RULE while_stmt = "while" "(" expression ")" statement ;
std::unique_ptr<WhileStmt> Parser::while_stmt() {
  if (!match({TOKEN_WHILE})) {
    return nullptr;
  }
  consume({TOKEN_LPAREN}, "Expected '(' after 'while'.");
  std::unique_ptr<Expr> condition = expression();
  if (!condition) {
    throw SyntaxError(current_token_, "Expected condition expression.");
  }
  consume({TOKEN_RPAREN}, "Expected ')' after while condition.");
  std::unique_ptr<Stmt> body = statement();
  if (!body) {
    throw SyntaxError(current_token_, "Expected body statement.");
  }

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

// RULE return_stmt = "return" [ expression ] ";" ;
std::unique_ptr<ReturnStmt> Parser::return_stmt() {
  if (!match({TOKEN_RETURN})) {
    return nullptr;
  }
  std::unique_ptr<Expr> value;
  if (!match({TOKEN_SEMICOLON})) {
    value = expression();
    if (!value) {
      throw SyntaxError(current_token_, "Expected expression after 'return'.");
    }
    consume({TOKEN_SEMICOLON}, "Expected ';' after returned expression.");
  }
  return std::make_unique<ReturnStmt>(std::move(value));
}

// RULE print_stmt = "print" expression ";" ;
std::unique_ptr<PrintStmt> Parser::print_stmt() {
  if (!match({TOKEN_PRINT})) {
    return nullptr;
  }
  std::unique_ptr<Expr> expr = expression();
  if (!expr) {
    throw SyntaxError(current_token_, "Expected expression after 'print'.");
  }
  consume({TOKEN_SEMICOLON}, "Expected ';' after printed expression.");
  return std::make_unique<PrintStmt>(std::move(expr));
}

// RULE inspect_stmt = "inspect" expression "{" { lambda_func } [ "default" "=>"
// statement ] "}" ;
std::unique_ptr<InspectStmt> Parser::inspect_stmt() {
  if (!match({TOKEN_INSPECT})) {
    return nullptr;
  }
  std::unique_ptr<Expr> inspected = expression();
  if (!inspected) {
    throw SyntaxError(current_token_, "Expected expression after 'inspect'.");
  }
  consume({TOKEN_LBRACE}, "Expected '{' after inspected expression.");
  std::vector<std::unique_ptr<LambdaFuncStmt>> lambdas;
  while (auto lambda = lambda_func()) {
    lambdas.push_back(std::move(lambda));
  }

  std::unique_ptr<Stmt> default_lambda;
  if (match({TOKEN_DEFAULT})) {
    consume({TOKEN_ARROW}, "Expected '=>' after default lambda.");
    default_lambda = statement();
    if (!default_lambda) {
      throw SyntaxError(current_token_, "Expected statement after '=>'.");
    }
  }
  consume({TOKEN_RBRACE}, "Expected '}' after inspect lambdas.");
  return std::make_unique<InspectStmt>(std::move(inspected), std::move(lambdas),
                                       std::move(default_lambda));
}

// RULE lambda_func = type identifier "=>" statement
std::unique_ptr<LambdaFuncStmt> Parser::lambda_func() {
  std::optional<Token> lambda_type = type();
  if (!lambda_type) {
    return nullptr;
  }
  Token lambda_id =
      consume({TOKEN_IDENTIFIER}, "Expected identifier after lambda type.");
  consume({TOKEN_ARROW}, "Expected '=>' after lambda identifier.");
  std::unique_ptr<Stmt> lambda_body = statement();
  if (!lambda_body) {
    throw SyntaxError(current_token_, "Expected statement for lambda body.");
  }
  return std::make_unique<LambdaFuncStmt>(lambda_type.value(), lambda_id,
                                          std::move(lambda_body));
}

// RULE block_stmt = "{" { statement } "}" ;
std::unique_ptr<BlockStmt> Parser::block_stmt() {
  if (!match({TOKEN_LBRACE})) {
    return nullptr;
  }
  std::vector<std::unique_ptr<Stmt>> statements;
  while (auto stmt = statement()) {
    statements.push_back(std::move(stmt));
  }
  consume({TOKEN_RBRACE}, "Expected '}' after block statement.");
  return std::make_unique<BlockStmt>(std::move(statements));
}

// RULE struct_decl = "struct" identifier "{" { struct_field } "}" ;
std::unique_ptr<StructDeclStmt> Parser::struct_decl() {
  if (!match({TOKEN_STRUCT})) {
    return nullptr;
  }
  Token struct_id =
      consume({TOKEN_IDENTIFIER}, "Expected identifier after 'struct'.");
  consume({TOKEN_LBRACE}, "Expected '{' after struct identifier.");
  std::vector<std::unique_ptr<StructFieldStmt>> fields;
  while (auto field = struct_field()) {
    fields.push_back(std::move(field));
  }
  consume({TOKEN_RBRACE}, "Expected '}' after struct declaration.");
  return std::make_unique<StructDeclStmt>(struct_id, std::move(fields));
}

// RULE struct_field = [ "mut" ] type identifier ";" ;
std::unique_ptr<StructFieldStmt> Parser::struct_field() {
  bool is_mut = false;
  if (match({TOKEN_MUT})) {
    is_mut = true;
  }
  std::optional<Token> field_type = type();
  if (!field_type) {
    if (is_mut) {
      throw SyntaxError(current_token_, "Expected struct field type.");
    }
    return nullptr;
  }
  Token field_id = consume({TOKEN_IDENTIFIER},
                           "Expected identifier after struct field type.");
  consume({TOKEN_SEMICOLON}, "Expected ';' after struct field.");
  return std::make_unique<StructFieldStmt>(field_type.value(), field_id,
                                           is_mut);
}

// RULE variant_decl = "variant" identifier "{" variant_params "}" ";" ;
std::unique_ptr<VariantDeclStmt> Parser::variant_decl() {
  if (!match({TOKEN_VARIANT})) {
    return nullptr;
  }
  Token variant_id =
      consume({TOKEN_IDENTIFIER}, "Expected variant identifier.");
  consume({TOKEN_LBRACE}, "Expected '{' after identifier.");
  std::vector<Token> params;
  if (auto variantparams = variant_params()) {
    params = variantparams.value();
  } else {
    throw SyntaxError(current_token_, "Expected variant parameters.");
  }
  consume({TOKEN_RBRACE}, "Expected '}' after variant parameters.");
  consume({TOKEN_SEMICOLON}, "Expected ';' after variant declaration.");
  return std::make_unique<VariantDeclStmt>(variant_id, std::move(params));
}

// RULE variant_params = type { "," type } ;
std::optional<std::vector<Token>> Parser::variant_params() {
  std::vector<Token> params;

  if (auto type_token = type()) {
    params.push_back(type_token.value());
  } else {
    return std::nullopt;
  }

  while (match({TOKEN_COMMA})) {
    auto type_token = type();
    if (!type_token) {
      throw SyntaxError(current_token_, "Expected variant parameter type.");
    }
    params.push_back(type_token.value());
  }
  return params;
}

// RULE var_or_func = mut_var_decl
//                  | void_func_decl
//                  | type var_or_func_decl
//                  | identifier assign_or_call;
std::unique_ptr<Stmt> Parser::var_or_func() {
  std::vector<std::function<std::unique_ptr<Stmt>()>> declaration_handlers = {
      [this]() { return mut_var_decl(); },
      [this]() { return void_func_decl(); }};

  for (auto& handler : declaration_handlers) {
    if (auto stmt = handler()) {
      return stmt;
    }
  }

  if (auto token = match({TOKEN_IDENTIFIER})) {
    if (auto assigncall = assign_or_call(token.value())) {
      return assigncall;
    }
    if (auto varfuncdecl = var_or_func_decl(token.value())) {
      return varfuncdecl;
    }
    throw SyntaxError(current_token_,
                      "Expected assignment, call or declaration.");
  }

  std::optional<Token> decl_type = type();
  if (decl_type) {
    if (auto varfuncdecl = var_or_func_decl(decl_type.value())) {
      return varfuncdecl;
    }
  }

  return nullptr;
}

// RULE assign_or_call = ( assign_stmt | call_stmt ) ;
std::unique_ptr<Stmt> Parser::assign_or_call(const Token& identifier) {
  if (auto call = call_stmt(identifier)) {
    return call;
  }
  auto id_expr = std::make_unique<VarExpr>(identifier);
  if (auto assign = assign_stmt(std::move(id_expr))) {
    return assign;
  }
  return nullptr;
}

// RULE assign = [ "." field_access ] "=" expression ";" ;
std::unique_ptr<AssignStmt> Parser::assign_stmt(std::unique_ptr<Expr> var) {
  if (match({TOKEN_DOT})) {
    var = field_access(std::move(var));
    consume({TOKEN_EQUAL}, "Expected '=' after field access for assignment.");
  } else {
    if (!match({TOKEN_EQUAL})) {
      return nullptr;
    }
  }

  auto value = expression();
  if (!value) {
    throw SyntaxError(current_token_, "Expected expression for assignment.");
  }
  consume({TOKEN_SEMICOLON}, "Expected ';' after assignment.");
  return std::make_unique<AssignStmt>(std::move(var), std::move(value));
}

// RULE call_stmt = "(" [ arguments ] ");" ;
std::unique_ptr<CallStmt> Parser::call_stmt(const Token& identifier) {
  if (!match({TOKEN_LPAREN})) {
    return nullptr;
  }
  if (match({TOKEN_RPAREN})) {
    consume({TOKEN_SEMICOLON}, "Expected ';' after call statement.");
    return std::make_unique<CallStmt>(identifier);
  }

  std::vector<std::unique_ptr<Expr>> call_args;
  if (!match({TOKEN_RPAREN})) {
    if (auto args = arguments()) {
      call_args = std::move(args.value());
    } else {
      throw SyntaxError(current_token_, "Expected call arguments.");
    }
    consume({TOKEN_RPAREN}, "Excepted ')' after call arguments.");
  }
  consume({TOKEN_SEMICOLON}, "Expected ';' after call statement.");
  return std::make_unique<CallStmt>(identifier, std::move(call_args));
}

// RULE var_or_func_decl = identifier ( var_decl | func_decl ) ;
std::unique_ptr<Stmt> Parser::var_or_func_decl(const Token& type) {
  if (auto identifier = match({TOKEN_IDENTIFIER})) {
    if (auto vardecl = var_decl(type, identifier.value(), false)) {
      return vardecl;
    }
    if (auto funcdecl = func_decl(type, identifier.value())) {
      return funcdecl;
    }
  }
  return nullptr;
}

// RULE mut_var_decl = "mut" type identifier var_decl ;
std::unique_ptr<VarDeclStmt> Parser::mut_var_decl() {
  if (!match({TOKEN_MUT})) {
    return nullptr;
  }
  auto var_type = type();
  if (!var_type) {
    throw SyntaxError(current_token_, "Expected variable type.");
  }
  Token identifier =
      consume({TOKEN_IDENTIFIER}, "Expected identifier after variable type.");
  if (auto vardecl = var_decl(var_type.value(), identifier, true)) {
    return vardecl;
  }
  throw SyntaxError(current_token_, "Expected variable declaration.");
}

// RULE void_func_decl = "void" identifier func_decl ;
std::unique_ptr<FuncStmt> Parser::void_func_decl() {
  if (auto return_type = match({TOKEN_VOID})) {
    Token identifier = consume(
        {TOKEN_IDENTIFIER}, "Expected identifier after function return type.");
    if (auto funcdecl = func_decl(return_type.value(), identifier)) {
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
  if (!match({TOKEN_EQUAL})) {
    return nullptr;
  }
  std::unique_ptr<Expr> expr = expression();
  if (!expr) {
    throw SyntaxError(current_token_, "Expected expression.");
  }
  consume({TOKEN_SEMICOLON}, "Expected ';' after variable declaration.");
  return std::make_unique<VarDeclStmt>(type, identifier, std::move(expr), mut);
}

// RULE func_decl = "(" [ func_params ] ")" block ;
std::unique_ptr<FuncStmt> Parser::func_decl(const Token& return_type,
                                            const Token& identifier) {
  if (!match({TOKEN_LPAREN})) {
    return nullptr;
  }
  std::vector<std::unique_ptr<FuncParamStmt>> params;
  if (!match({TOKEN_RPAREN})) {
    if (auto funcparams = func_params()) {
      params = std::move(funcparams.value());
    } else {
      throw SyntaxError(current_token_, "Expected function parameters.");
    }
    consume({TOKEN_RPAREN}, "Excepted ')' after function parameters.");
  }
  auto body = block_stmt();
  if (!body) {
    throw SyntaxError(current_token_,
                      "Expected block statement in function declaration.");
  }
  return std::make_unique<FuncStmt>(identifier, return_type, std::move(params),
                                    std::move(body));
}

// RULE func_params = type identifier { "," type identifier } ;
std::optional<std::vector<std::unique_ptr<FuncParamStmt>>>
Parser::func_params() {
  std::vector<std::unique_ptr<FuncParamStmt>> params;

  if (auto param_type = type()) {
    Token param_id =
        consume({TOKEN_IDENTIFIER}, "Expected identifier after type.");
    params.push_back(
        std::make_unique<FuncParamStmt>(param_type.value(), param_id));
  } else {
    return std::nullopt;
  }

  while (match({TOKEN_COMMA})) {
    std::optional<Token> param_type = type();
    if (!param_type) {
      throw SyntaxError(current_token_, "Expected function parameter type.");
    }
    Token param_id =
        consume({TOKEN_IDENTIFIER}, "Expected identifier after type.");
    params.push_back(
        std::make_unique<FuncParamStmt>(param_type.value(), param_id));
  }
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

  while (auto token = match({TOKEN_OR})) {
    std::unique_ptr<Expr> right = logic_and();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    expr = std::make_unique<LogicalExpr>(std::move(expr), op_symbol,
                                         std::move(right));
  }

  return expr;
}

// RULE logic_and = equality { "and" equality } ;
std::unique_ptr<Expr> Parser::logic_and() {
  std::unique_ptr<Expr> expr = equality();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match({TOKEN_AND})) {
    std::unique_ptr<Expr> right = equality();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    expr = std::make_unique<LogicalExpr>(std::move(expr), op_symbol,
                                         std::move(right));
  }

  return expr;
}

// RULE equality = comparison { ( "!=" | "==" ) comparison } ;
std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match({TOKEN_NOT_EQUAL, TOKEN_EQUAL_EQUAL})) {
    std::unique_ptr<Expr> right = comparison();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE comparison = term { ( ">" | ">=" | "<" | "<=" ) term } ;
std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match({TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS,
                             TOKEN_LESS_EQUAL})) {
    std::unique_ptr<Expr> right = term();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE term = factor { ( "-" | "+" ) factor } ;
std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match({TOKEN_MINUS, TOKEN_PLUS})) {
    std::unique_ptr<Expr> right = factor();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE factor = unary { ( "/" | "*" ) unary } ;
std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match({TOKEN_SLASH, TOKEN_STAR})) {
    std::unique_ptr<Expr> right = unary();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE unary = [ "!" | "-" ] type_cast ;
std::unique_ptr<Expr> Parser::unary() {
  if (auto token = match({TOKEN_EXCLAMATION, TOKEN_MINUS})) {
    std::unique_ptr<Expr> right = type_cast();
    if (!right) {
      throw SyntaxError(current_token_, "Expected expression.");
    }
    Token op_symbol = token.value();
    return std::make_unique<UnaryExpr>(op_symbol, std::move(right));
  }

  return type_cast();
}

// RULE type_cast = call { ("as" | "is") type } ;
std::unique_ptr<Expr> Parser::type_cast() {
  std::unique_ptr<Expr> expr = call();

  if (!expr) {
    return nullptr;
  }

  while (auto token = match({TOKEN_AS, TOKEN_IS})) {
    Token op_symbol = token.value();
    auto cast_type = type();
    if (!cast_type) {
      throw SyntaxError(current_token_, "Expected cast type.");
    }
    expr = std::make_unique<CastExpr>(std::move(expr), op_symbol,
                                      cast_type.value());
  }

  return expr;
}

// RULE call = primary { "(" [ arguments ] ")" | field_access };
std::unique_ptr<Expr> Parser::call() {
  std::unique_ptr<Expr> expr = primary();

  if (!expr) {
    return nullptr;
  }

  if (match({TOKEN_LPAREN})) {
    std::vector<std::unique_ptr<Expr>> call_args;
    if (!match({TOKEN_RPAREN})) {
      auto args = arguments();
      if (!args) {
        throw SyntaxError(current_token_, "Expected arguments in call.");
      }
      call_args = std::move(args.value());
      consume({TOKEN_RPAREN}, "Excepted ')' after call arguments.");
    }
    expr = std::make_unique<CallExpr>(std::move(expr), std::move(call_args));
  } else if (match({TOKEN_DOT})) {
    expr = field_access(std::move(expr));
  }

  return expr;
}

// RULE primary = string | int_val | float_val | bool_values | identifier | "("
// expression ")" | "{" arguments "}" ;
std::unique_ptr<Expr> Parser::primary() {
  if (auto token = match({TOKEN_FLOAT_VAL, TOKEN_INT_VAL, TOKEN_STR_VAL,
                          TOKEN_TRUE, TOKEN_FALSE})) {
    return std::make_unique<LiteralExpr>(token.value());
  }

  if (auto token = match({TOKEN_IDENTIFIER})) {
    Token identifier = token.value();
    return std::make_unique<VarExpr>(identifier);
  }

  if (match({TOKEN_LPAREN})) {
    std::unique_ptr<Expr> expr = expression();
    if (!expr) {
      throw SyntaxError(current_token_, "Expected expression after '('.");
    }
    consume({TOKEN_RPAREN}, "Excepted ')' after expression.");
    return std::make_unique<GroupingExpr>(std::move(expr));
  }

  if (match({TOKEN_LBRACE})) {
    auto args = arguments();
    if (!args) {
      throw SyntaxError(current_token_,
                        "Expected arguments for initalizer list.");
    }
    consume({TOKEN_RBRACE}, "Excepted '}' after initializer list.");
    return std::make_unique<InitalizerListExpr>(std::move(args.value()));
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

  if (match({TOKEN_COMMA})) {
    while (auto expr = expression()) {
      if (args.size() > MAX_ARGUMENTS) {
        throw SyntaxError(current_token_, "Maximum amount (" +
                                              std::to_string(MAX_ARGUMENTS) +
                                              ") of arguments exceeded.");
      }
      args.push_back(std::move(expr));
      if (!match({TOKEN_COMMA})) {
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
    auto id = consume({TOKEN_IDENTIFIER},
                      "Expected identifier after '.' for accessing field.");
    parent_struct =
        std::make_unique<FieldAccessExpr>(std::move(parent_struct), id);
  } while (match({TOKEN_DOT}));
  return parent_struct;
}

// RULE type = "bool" | "str" | "int" | "float" | identifier ;
std::optional<Token> Parser::type() {
  if (auto token = match(
          {TOKEN_FLOAT, TOKEN_INT, TOKEN_STR, TOKEN_BOOL, TOKEN_IDENTIFIER})) {
    return token.value();
  }

  return std::nullopt;
}

opt_token_t Parser::match(std::initializer_list<TokenType> types) {
  for (const auto& type : types) {
    if (current_token_.get_type() == type) {
      auto prev = current_token_;
      advance();
      return prev;
    }
  }
  return std::nullopt;
}

Token Parser::advance() { return current_token_ = lexer_.next_token(); }

Token Parser::consume(std::initializer_list<TokenType> types,
                      const std::string& err_msg) {
  if (auto token = match(types)) {
    return token.value();
  }
  throw SyntaxError(current_token_, err_msg);
}
