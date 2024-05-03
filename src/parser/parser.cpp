#include "parser.hpp"

#include <algorithm>

std::vector<std::unique_ptr<Expr>> Parser::parse() {
  std::vector<std::unique_ptr<Expr>> statements;
  while (current_token_.get_type() != TokenType::TOKEN_ETX) {
    statements.push_back(expression());
  }
  return statements;
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

// RULE equality	= comparison { ( "!=" | "==" ) comparison } ;
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
    std::unique_ptr<Expr> right = type();
    Token op_symbol = token.value();
    expr = std::make_unique<CastExpr>(std::move(expr), op_symbol,
                                      std::move(right));
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
std::unique_ptr<Expr> Parser::type() {
  if (auto token = match({TokenType::TOKEN_FLOAT, TokenType::TOKEN_INT,
                          TokenType::TOKEN_STR, TokenType::TOKEN_BOOL,
                          TokenType::TOKEN_IDENTIFIER})) {
    return std::make_unique<TypeExpr>(token.value());
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
