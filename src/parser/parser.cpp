#include "parser.hpp"

#include <algorithm>

// RULE program = { declaration } ;
std::unique_ptr<Program> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!check({TokenType::TOKEN_ETX})) {
    statements.push_back(declaration());
  }
  return std::make_unique<Program>(std::move(statements));
}

// RULE declaration	= assign_call_decl
//                  | struct_decl
//                  | variant_decl
//                  | statement ;
std::unique_ptr<Stmt> Parser::declaration() {
  switch (current_token_.get_type()) {
    case TOKEN_IDENTIFIER:
    case TOKEN_MUT:
    case TOKEN_VOID:
    case TOKEN_BOOL:
    case TOKEN_STR:
    case TOKEN_INT:
    case TOKEN_FLOAT:
      return assign_call_decl();
    case TOKEN_STRUCT:
      return struct_decl();
    case TOKEN_VARIANT:
      return variant_decl();
    default:
      return statement();
  }
}

// RULE assign_call_decl= mut_var_decl
//                      | void_func_decl
//                      | var_func_decl
//                      | assign_call;
std::unique_ptr<Stmt> Parser::assign_call_decl() {
  if (check({TOKEN_MUT})) {
    return mut_var_decl();
  }
  if (check({TOKEN_VOID})) {
    // void_func_decl
    throw SyntaxError(current_token_, "UNIMPLEMENTED.");
  }
  if (auto token = match({TOKEN_IDENTIFIER})) {
    // assign_call ( field_access ./=/( ), var_func_decl (identifier) )
    if (check({TOKEN_IDENTIFIER})) {
      return var_func_decl(token.value());
    }
    return assign_call(token.value());
  }

  // expect type - var_func_decl
  return var_func_decl(type());
}

// RULE assign_call = identifier ( assign | call )
std::unique_ptr<Stmt> Parser::assign_call(const Token& identifier) {
  if (check({TOKEN_LPAREN})) {
    return call_stmt(identifier);
  }
  auto id_expr = std::make_unique<VarExpr>(identifier.stringify());
  return assign_stmt(std::move(id_expr));
}

// RULE assign = [ field_access ] "=" expression ";" ;
std::unique_ptr<AssignStmt> Parser::assign_stmt(std::unique_ptr<VarExpr> var) {
  auto field_var = field_access(std::move(var));
  consume({TOKEN_EQUAL}, "Expected '=' after identifier.");
  auto value = expression();
  consume({TOKEN_SEMICOLON}, "Expected ';' after expression.");
  return std::make_unique<AssignStmt>(std::move(field_var), std::move(value));
}

// RULE call_stmt = "(" [ arguments ] ");" ;
std::unique_ptr<CallStmt> Parser::call_stmt(const Token &identifier) {
  consume({TokenType::TOKEN_LPAREN},
          "Excepted '(' before function arguments.");
  if (match({TOKEN_RPAREN})) {
    return std::make_unique<CallStmt>(identifier.stringify());
  }

  auto args = arguments();
  consume({TokenType::TOKEN_RPAREN},
          "Excepted ')' after function arguments.");
  consume({TOKEN_SEMICOLON}, "Expected ';' after expression.");
  return std::make_unique<CallStmt>(identifier.stringify(), std::move(args));
}

// RULE var_func_decl = type identifier ( var_decl | func_decl )
std::unique_ptr<Stmt> Parser::var_func_decl(Token type) {
  Token identifier =
      consume({TOKEN_IDENTIFIER},
              "Expected identifier after type.");
  if (check({TOKEN_EQUAL})) {
    return var_decl(std::move(type), identifier.stringify(), false);
  } else {
    throw SyntaxError(current_token_, "UNIMPLEMENTED.");
  }
}

// RULE mut_var_decl = "mut" type identifier var_decl ;
std::unique_ptr<VarDeclStmt> Parser::mut_var_decl() {
  consume({TOKEN_MUT}, "Expected 'mut'.");
  auto var_type = type();
  Token identifier =
      consume({TOKEN_IDENTIFIER},
              "Expected identifier after type.");
  return var_decl(var_type, identifier.stringify(), true);
}

// RULE var_decl = "=" expression ";" ;
std::unique_ptr<VarDeclStmt> Parser::var_decl(Token type,
                                              std::string identifier,
                                              bool mut) {
  consume({TOKEN_EQUAL}, "Expected '=' for assign statement");
  std::unique_ptr<Expr> expr = expression();
  consume({TOKEN_SEMICOLON}, "Expected ';' after assignment.");
  return std::make_unique<VarDeclStmt>(std::move(type), std::move(identifier),
                                       std::move(expr), mut);
}

// RULE struct_decl = "struct" identifier "{" { struct_field } "}" ;
std::unique_ptr<StructDeclStmt> Parser::struct_decl() {
  consume({TOKEN_STRUCT}, "Expected 'struct' for struct decl statement.");
  Token struct_id = consume({TOKEN_IDENTIFIER},"Expected identifier after 'struct'.");
  consume({TOKEN_LBRACE}, "Expected '{' after struct identifier.");
  std::vector<std::unique_ptr<StructFieldStmt>> fields;
  while(!check({TOKEN_RBRACE, TOKEN_ETX})){
    fields.push_back(struct_field());
  }
  consume({TOKEN_RBRACE}, "Expected '}' after fields.");
  return std::make_unique<StructDeclStmt>(struct_id.stringify(), std::move(fields));
}

// RULE struct_field = [ "mut" ] type identifier ";" ;
std::unique_ptr<StructFieldStmt> Parser::struct_field() {
  bool is_mut = false;
  if (match({TOKEN_MUT})) {
    is_mut = true;
  }
  Token field_type = type();
  Token field_id = consume({TOKEN_IDENTIFIER},"Expected identifier after type.");
  consume({TOKEN_SEMICOLON}, "Expected ';' after statement.");
  return std::make_unique<StructFieldStmt>(field_type, field_id.stringify(), is_mut);
}

// RULE variant_decl = "variant" identifier "{" type { "," type } "}" ";" ;
std::unique_ptr<VariantDeclStmt> Parser::variant_decl() {
  consume({TOKEN_VARIANT}, "Expected 'variant' for variant decl statement.");
  Token variant_id = consume({TOKEN_IDENTIFIER},"Expected identifier after 'variant'.");
  consume({TOKEN_LBRACE}, "Expected '{' after variant identifier.");
  std::vector<Token> variant_params;
  do {
    variant_params.push_back(type());
  } while (match({TOKEN_COMMA}));
  consume({TOKEN_RBRACE}, "Expected '}' after variant types.");
  consume({TOKEN_SEMICOLON}, "Expected ';' after statement.");
  return std::make_unique<VariantDeclStmt>(variant_id.stringify(), std::move(variant_params));
}

// RULE statement = if_stmt
//                | while_stmt
//                | return_stmt
//                | print_stmt
//                | inspect_stmt
//                | block_stmt ;
std::unique_ptr<Stmt> Parser::statement() {
  switch (current_token_.get_type()) {
    case TOKEN_PRINT:
      return print_stmt();
    case TOKEN_IF:
      return if_stmt();
    case TOKEN_LBRACE:
      return block_stmt();
    case TOKEN_WHILE:
      return while_stmt();
    default:
      throw SyntaxError(current_token_, "Expected statement.");
  }
}

// RULE print_stmt = "print" expression ";" ;
std::unique_ptr<PrintStmt> Parser::print_stmt() {
  consume({TOKEN_PRINT}, "Expected 'print' for print statement.");
  std::unique_ptr<Expr> expr = expression();
  consume({TOKEN_SEMICOLON},
          "Expected ';' after printed expression.");
  return std::make_unique<PrintStmt>(std::move(expr));
}

// RULE if_stmt = "if" "(" expression ")" statement [ "else" statement ] ;
std::unique_ptr<IfStmt> Parser::if_stmt() {
  consume({TOKEN_IF}, "Expected 'if' keyword for if statement.");
  consume({TOKEN_LPAREN}, "Expected '(' after 'if'.");
  std::unique_ptr<Expr> condition = expression();
  consume({TOKEN_RPAREN}, "Expected ')' after condition.");
  std::unique_ptr<Stmt> then_branch = statement();
  std::unique_ptr<Stmt> else_branch;
  if (match({TOKEN_ELSE})) {
    else_branch = statement();
  }
  return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch),
                                  std::move(else_branch));
}

// RULE block = "{" { declaration } "}" ;
std::unique_ptr<BlockStmt> Parser::block_stmt() {
  consume({TOKEN_LBRACE}, "Expected '{' before block statement.");
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!check({TOKEN_RBRACE, TOKEN_ETX})) {
    statements.push_back(declaration());
  }
  consume({TOKEN_RBRACE}, "Expected '}' after block statement.");
  return std::make_unique<BlockStmt>(std::move(statements));
}

// RULE while_stmt = "while" "(" expression ")" statement ;
std::unique_ptr<WhileStmt> Parser::while_stmt() {
  consume({TOKEN_WHILE},
          "Expected 'while' keyword for while statement.");
  consume({TOKEN_LPAREN}, "Expected '(' after 'while'.");
  std::unique_ptr<Expr> condition = expression();
  consume({TOKEN_RPAREN}, "Expected ')' after while condition.");
  std::unique_ptr<Stmt> body = statement();

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

// RULE expression = logic_or ;
std::unique_ptr<Expr> Parser::expression() { return logic_or(); }

// RULE logic_or = logic_and { "or" logic_and } ;
std::unique_ptr<Expr> Parser::logic_or() {
  std::unique_ptr<Expr> expr = logic_and();

  while (auto token = match({TOKEN_OR})) {
    std::unique_ptr<Expr> right = logic_and();
    Token op_symbol = token.value();
    expr = std::make_unique<LogicalExpr>(std::move(expr), op_symbol,
                                         std::move(right));
  }

  return expr;
}

// RULE logic_and = equality { "and" equality } ;
std::unique_ptr<Expr> Parser::logic_and() {
  std::unique_ptr<Expr> expr = equality();

  while (auto token = match({TOKEN_AND})) {
    std::unique_ptr<Expr> right = equality();
    Token op_symbol = token.value();
    expr = std::make_unique<LogicalExpr>(std::move(expr), op_symbol,
                                         std::move(right));
  }

  return expr;
}

// RULE equality = comparison { ( "!=" | "==" ) comparison } ;
std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();

  while (auto token = match(
             {TOKEN_NOT_EQUAL, TOKEN_EQUAL_EQUAL})) {
    std::unique_ptr<Expr> right = comparison();
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE comparison = term { ( ">" | ">=" | "<" | "<=" ) term } ;
std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  while (auto token =
             match({TOKEN_GREATER, TOKEN_GREATER_EQUAL,
                    TOKEN_LESS, TOKEN_LESS_EQUAL})) {
    std::unique_ptr<Expr> right = term();
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE term = factor { ( "-" | "+" ) factor } ;
std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  while (auto token = match({TOKEN_MINUS, TOKEN_PLUS})) {
    std::unique_ptr<Expr> right = factor();
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE factor = unary { ( "/" | "*" ) unary } ;
std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  while (auto token = match({TOKEN_SLASH, TOKEN_STAR})) {
    std::unique_ptr<Expr> right = unary();
    Token op_symbol = token.value();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op_symbol,
                                        std::move(right));
  }

  return expr;
}

// RULE unary = ("!" | "-" ) type_cast ;
std::unique_ptr<Expr> Parser::unary() {
  if (auto token =
          match({TOKEN_EXCLAMATION, TOKEN_MINUS})) {
    std::unique_ptr<Expr> right = type_cast();
    Token op_symbol = token.value();
    return std::make_unique<UnaryExpr>(op_symbol, std::move(right));
  }

  return type_cast();
}

// RULE type_cast = call { ("as" | "is") type } ;
std::unique_ptr<Expr> Parser::type_cast() {
  std::unique_ptr<Expr> expr = call();

  while (auto token = match({TOKEN_AS, TOKEN_IS})) {
    Token op_symbol = token.value();
    expr = std::make_unique<CastExpr>(std::move(expr), op_symbol, type());
  }

  return expr;
}

// RULE call = primary { "(" [ arguments ] ")" | "." identifier };
std::unique_ptr<Expr> Parser::call() {
  std::unique_ptr<Expr> expr = primary();

  if (match({TOKEN_LPAREN})) {
    if (match({TOKEN_RPAREN})) {
      expr = std::make_unique<CallExpr>(std::move(expr));
    } else {
      auto args = arguments();
      consume({TOKEN_RPAREN},
              "Excepted ')' after function arguments.");
      expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
    }
  } else if (check({TOKEN_DOT})) {
    expr = field_access(std::move(expr));
  }

  return expr;
}

// RULE primary = string | int_val | float_val | bool_values | identifier | "("
// expression ")" | "{" arguments "}" ;
std::unique_ptr<Expr> Parser::primary() {
  if (auto token = match({TOKEN_FLOAT_VAL, TOKEN_INT_VAL,
                          TOKEN_STR_VAL, TOKEN_TRUE,
                          TOKEN_FALSE})) {
    return std::make_unique<LiteralExpr>(token.value());
  }

  if (auto token = match({TOKEN_IDENTIFIER})) {
    std::string identifier = token.value().stringify();
    return std::make_unique<VarExpr>(identifier);
  }

  if (match({TOKEN_LPAREN})) {
    std::unique_ptr<Expr> expr = expression();
    consume({TOKEN_RPAREN}, "Excepted ')' after expression.");
    return std::make_unique<GroupingExpr>(std::move(expr));
  }

  if (match({TOKEN_LBRACE})) {
    auto exprs = arguments();
    consume({TOKEN_RBRACE}, "Excepted '}' after initializer list.");
    return std::make_unique<InitalizerListExpr>(std::move(exprs));
  }

  throw SyntaxError(current_token_, "Expected expression.");
}

// RULE type = "bool" | "str" | "int" | "float" | identifier ;
Token Parser::type() {
  if (auto token = match({TOKEN_FLOAT, TOKEN_INT,
                          TOKEN_STR, TOKEN_BOOL,
                          TOKEN_IDENTIFIER})) {
    return token.value();
  }

  throw SyntaxError(current_token_, "Expected type.");
}

// RULE arguments = expression { "," expression } ;
std::vector<std::unique_ptr<Expr>> Parser::arguments() {
  std::vector<std::unique_ptr<Expr>> args;

  do {
    if (args.size() > MAX_ARGUMENTS) {
      throw SyntaxError(current_token_, "Maximum amount (" +
                                            std::to_string(MAX_ARGUMENTS) +
                                            ") of arguments exceeded.");
    }
    args.push_back(expression());
  } while (match({TOKEN_COMMA}));
  return args;
}

// RULE field_access = { "." identifier } ;
std::unique_ptr<Expr> Parser::field_access(
    std::unique_ptr<Expr> parent_struct) {
  while (match({TOKEN_DOT})) {
    auto id = consume({TOKEN_IDENTIFIER},
                      "Expected identifier after '.' for accessing field.");
    parent_struct =
        std::make_unique<FieldAccessExpr>(std::move(parent_struct), id);
  }
  return parent_struct;
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

bool Parser::check(std::initializer_list<TokenType> types) {
  return std::ranges::any_of(types, [&](const auto& type) {
    return current_token_.get_type() == type;
  });
}
