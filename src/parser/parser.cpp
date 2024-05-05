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
    // assign_call ( field_access ./=/( ), var_func_decl (identifier)
    if (check({TOKEN_IDENTIFIER})) {
      return var_func_decl(token.value());
    }
    throw SyntaxError(current_token_, "UNIMPLEMENTED.");
  }

  // expect type - var_func_decl
  return var_func_decl(type());
}

// RULE var_func_decl = type identifier ( var_decl | func_decl )
std::unique_ptr<Stmt> Parser::var_func_decl(Token type) {
  Token identifier =
      consume({TOKEN_IDENTIFIER},
              "Expected identifier after type in var func statement.");
  if (check({TOKEN_EQUAL})) {
    return var_decl(std::move(type), identifier.stringify(), false);
  } else {
    throw SyntaxError(current_token_, "UNIMPLEMENTED.");
  }
}

// RULE mut_var_decl = "mut" type identifier var_decl ;
std::unique_ptr<VarDeclStmt> Parser::mut_var_decl() {
  consume({TOKEN_MUT}, "Expected 'mut' for mut var decl statement.");
  auto var_type = type();
  Token identifier =
      consume({TOKEN_IDENTIFIER},
              "Expected identifier after type in mut var decl statement.");
  return var_decl(var_type, identifier.stringify(), true);
}

// RULE var_decl = "=" expression ";" ;
std::unique_ptr<VarDeclStmt> Parser::var_decl(Token type,
                                              std::string identifier,
                                              bool mut) {
  consume({TOKEN_EQUAL}, "Expected '=' for assign statement");
  std::unique_ptr<Expr> expr = expression();
  consume({TokenType::TOKEN_SEMICOLON}, "Expected ';' after assign statement.");
  return std::make_unique<VarDeclStmt>(std::move(type), std::move(identifier),
                                       std::move(expr), mut);
}

// RULE statement = if_stmt
//                | while_stmt
//                | return_stmt
//                | print_stmt
//                | inspect_stmt
//                | block_stmt ;
std::unique_ptr<Stmt> Parser::statement() {
  switch (current_token_.get_type()) {
    case TokenType::TOKEN_PRINT:
      return print_stmt();
    case TokenType::TOKEN_IF:
      return if_stmt();
    case TokenType::TOKEN_LBRACE:
      return block_stmt();
    case TokenType::TOKEN_WHILE:
      return while_stmt();
    default:
      throw SyntaxError(current_token_, "Expected statement.");
  }
}

// RULE print_stmt = "print" expression ";" ;
std::unique_ptr<PrintStmt> Parser::print_stmt() {
  consume({TokenType::TOKEN_PRINT}, "Expected 'print' for print statement.");
  std::unique_ptr<Expr> expr = expression();
  consume({TokenType::TOKEN_SEMICOLON},
          "Expected ';' after printed expression.");
  return std::make_unique<PrintStmt>(std::move(expr));
}

// RULE if_stmt = "if" "(" expression ")" statement [ "else" statement ] ;
std::unique_ptr<IfStmt> Parser::if_stmt() {
  consume({TokenType::TOKEN_IF}, "Expected 'if' keyword for if statement.");
  consume({TokenType::TOKEN_LPAREN}, "Expected '(' after 'if'.");
  std::unique_ptr<Expr> condition = expression();
  consume({TokenType::TOKEN_RPAREN}, "Expected ')' after if condition.");
  std::unique_ptr<Stmt> then_branch = statement();
  std::unique_ptr<Stmt> else_branch;
  if (match({TokenType::TOKEN_ELSE})) {
    else_branch = statement();
  }
  return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch),
                                  std::move(else_branch));
}

// RULE block = "{" { declaration } "}" ;
std::unique_ptr<BlockStmt> Parser::block_stmt() {
  consume({TokenType::TOKEN_LBRACE}, "Expected '{' before block statement.");
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!check({TokenType::TOKEN_RBRACE, TokenType::TOKEN_ETX})) {
    statements.push_back(declaration());
  }
  consume({TokenType::TOKEN_RBRACE}, "Expected '}' after block statement.");
  return std::make_unique<BlockStmt>(std::move(statements));
}

// RULE while_stmt = "while" "(" expression ")" statement ;
std::unique_ptr<WhileStmt> Parser::while_stmt() {
  consume({TokenType::TOKEN_WHILE},
          "Expected 'while' keyword for while statement.");
  consume({TokenType::TOKEN_LPAREN}, "Expected '(' after 'while'.");
  std::unique_ptr<Expr> condition = expression();
  consume({TokenType::TOKEN_RPAREN}, "Expected ')' after while condition.");
  std::unique_ptr<Stmt> body = statement();

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

// RULE expression = logic_or ;
std::unique_ptr<Expr> Parser::expression() { return logic_or(); }

// RULE logic_or = logic_and { "or" logic_and } ;
std::unique_ptr<Expr> Parser::logic_or() {
  std::unique_ptr<Expr> expr = logic_and();

  while (auto token = match({TokenType::TOKEN_OR})) {
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

  while (auto token = match({TokenType::TOKEN_AND})) {
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
             {TokenType::TOKEN_NOT_EQUAL, TokenType::TOKEN_EQUAL_EQUAL})) {
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
             match({TokenType::TOKEN_GREATER, TokenType::TOKEN_GREATER_EQUAL,
                    TokenType::TOKEN_LESS, TokenType::TOKEN_LESS_EQUAL})) {
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

  while (auto token = match({TokenType::TOKEN_MINUS, TokenType::TOKEN_PLUS})) {
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

  while (auto token = match({TokenType::TOKEN_SLASH, TokenType::TOKEN_STAR})) {
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
          match({TokenType::TOKEN_EXCLAMATION, TokenType::TOKEN_MINUS})) {
    std::unique_ptr<Expr> right = type_cast();
    Token op_symbol = token.value();
    return std::make_unique<UnaryExpr>(op_symbol, std::move(right));
  }

  return type_cast();
}

// RULE type_cast = call { ("as" | "is") type } ;
std::unique_ptr<Expr> Parser::type_cast() {
  std::unique_ptr<Expr> expr = call();

  while (auto token = match({TokenType::TOKEN_AS, TokenType::TOKEN_IS})) {
    Token op_symbol = token.value();
    expr = std::make_unique<CastExpr>(std::move(expr), op_symbol, type());
  }

  return expr;
}

// RULE call = primary { "(" [ arguments ] ")" | "." identifier };
std::unique_ptr<Expr> Parser::call() {
  std::unique_ptr<Expr> expr = primary();

  if (match({TokenType::TOKEN_LPAREN})) {
    auto args = arguments();
    consume({TokenType::TOKEN_RPAREN},
            "Excepted ')' after function arguments.");
    expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
  } else if (check({TokenType::TOKEN_DOT})) {
    expr = field_access(std::move(expr));
  }

  return expr;
}

// RULE primary = string | int_val | float_val | bool_values | identifier | "("
// expression ")" | "{" arguments "}" ;
std::unique_ptr<Expr> Parser::primary() {
  if (auto token = match({TokenType::TOKEN_FLOAT_VAL, TokenType::TOKEN_INT_VAL,
                          TokenType::TOKEN_STR_VAL, TokenType::TOKEN_TRUE,
                          TokenType::TOKEN_FALSE})) {
    return std::make_unique<LiteralExpr>(token.value());
  }

  if (auto token = match({TokenType::TOKEN_IDENTIFIER})) {
    std::string identifier = token.value().stringify();
    return std::make_unique<VarExpr>(identifier);
  }

  if (match({TokenType::TOKEN_LPAREN})) {
    std::unique_ptr<Expr> expr = expression();
    consume({TokenType::TOKEN_RPAREN}, "Excepted ')' after expression.");
    return std::make_unique<GroupingExpr>(std::move(expr));
  }

  if (match({TokenType::TOKEN_LBRACE})) {
    auto exprs = arguments();
    consume({TokenType::TOKEN_RBRACE}, "Excepted '}' after initializer list.");
    return std::make_unique<InitalizerListExpr>(std::move(exprs));
  }

  throw SyntaxError(current_token_, "Expected expression.");
}

// RULE type = "bool" | "str" | "int" | "float" | identifier ;
Token Parser::type() {
  if (auto token = match({TokenType::TOKEN_FLOAT, TokenType::TOKEN_INT,
                          TokenType::TOKEN_STR, TokenType::TOKEN_BOOL,
                          TokenType::TOKEN_IDENTIFIER})) {
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
  } while (match({TokenType::TOKEN_COMMA}));
  return args;
}

// RULE field_access = { "." identifier } ;
std::unique_ptr<Expr> Parser::field_access(
    std::unique_ptr<Expr> parent_struct) {
  while (match({TOKEN_DOT})) {
    auto id = consume({TokenType::TOKEN_IDENTIFIER},
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
