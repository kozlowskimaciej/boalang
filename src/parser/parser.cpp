#include "parser.hpp"

std::vector<std::unique_ptr<Expr>> Parser::parse() {
  std::vector<std::unique_ptr<Expr>> statements;
  while (current_token_.get_type() != TokenType::TOKEN_ETX) {
    statements.push_back(expression());
  }
  return statements;
}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
  return logic_or();
  //  std::unique_ptr<Expr> expr = logic_or();
  //
  //  if (match({TOKEN_EQUAL})) {
  //    Token equals = previous();
  //    Expr* value = assignment();
  //
  //    if (VarExpr* var_expr = dynamic_cast<VarExpr*>(expr)) {
  //      Token name = var_expr->name;
  //      return new AssignExpr(name, value);
  //    }
  //
  //    logic_error(equals, "Invalid assignment target.");
  //  }
  //
  //  return expr;
}

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

std::unique_ptr<Expr> Parser::unary() {
  if (auto token =
          match({TokenType::TOKEN_EXCLAMATION, TokenType::TOKEN_MINUS})) {
    std::unique_ptr<Expr> right = type_cast();
    Token op_symbol = token.value();
    return std::make_unique<UnaryExpr>(op_symbol, std::move(right));
  }

  return type_cast();
}

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

std::unique_ptr<Expr> Parser::call() {
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (auto token = match({TokenType::TOKEN_FLOAT_VAL, TokenType::TOKEN_INT_VAL,
                          TokenType::TOKEN_STR_VAL, TokenType::TOKEN_TRUE, TokenType::TOKEN_FALSE})) {
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
    std::vector<std::unique_ptr<Expr>> exprs;
    do {
      exprs.push_back(expression());
    } while(match({TokenType::TOKEN_COMMA}));
    consume({TokenType::TOKEN_RBRACE}, "Excepted '}' after initializer list.");
    return std::make_unique<InitalizerListExpr>(std::move(exprs));
  }

  throw SyntaxError(current_token_, "Expected expression.");
}

std::unique_ptr<Expr> Parser::type() {
  if (auto token = match({TokenType::TOKEN_FLOAT, TokenType::TOKEN_INT,
                          TokenType::TOKEN_STR, TokenType::TOKEN_BOOL, TokenType::TOKEN_IDENTIFIER})) {
    return std::make_unique<TypeExpr>(token.value());
  }

  throw SyntaxError(current_token_, "Expected type.");
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
