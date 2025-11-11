//
// Created by Nicolae Popescu on 11/11/2025.
//

#include "ast.hpp"

#include <string>
#include <utility>
#include <vector>

namespace toy {

using Token = Token;
/**
 *  Base class for all expression nodes.
 */
class ExpressionAST {
public:
  virtual ~ExpressionAST() = default;
};

/**
 * Expression class for numeric literals like "1.0".
 */
class NumberExpressionAST final : public ExpressionAST {
public:
  explicit NumberExpressionAST(double value) : value_{value} {}

private:
  double value_;
};

/**
 * Expression class for referencing a variable, like "a".
 */
class VariableExpressionAST final : public ExpressionAST {
public:
  explicit VariableExpressionAST(std::string name) : name_{std::move(name)} {}

private:
  std::string name_;
};

/**
 * Expression class for a binary operator like "+".
 */
class BinaryExpressionAST final : public ExpressionAST {
public:
  explicit BinaryExpressionAST(Token op, std::unique_ptr<ExpressionAST> lhs,
                               std::unique_ptr<ExpressionAST> rhs)
      : operator_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

private:
  Token operator_;
  std::unique_ptr<ExpressionAST> lhs_, rhs_;
};

/**
 * Expression class for function calls.
 */
class CallExpressionAST final : public ExpressionAST {
public:
  explicit CallExpressionAST(
      std::string callee, std::vector<std::unique_ptr<ExpressionAST>> arguments)
      : callee_{std::move(callee)}, arguments_{std::move(arguments)} {}

private:
  std::string callee_;
  std::vector<std::unique_ptr<ExpressionAST>> arguments_;
};

/**
 * This class represents the "prototype" for a function, which captures its
 * name, and its argument names (thus implicitly the number of arguments the
 * function takes).
 */
class FunctionPrototypeAST {
public:
  FunctionPrototypeAST(std::string name, std::vector<std::string> arguments)
      : name_{std::move(name)}, arguments_{std::move(arguments)} {}

private:
  std::string name_;
  std::vector<std::string> arguments_;
};

static void log_error(const char *token) {
  fprintf(stderr, "error: %s\n", token);
}

static void log_error_prototype(const char *token) { log_error(token); }

/**
 * This class represents a function definition itself.
 */
class FunctionDefinitionAST {
public:
  FunctionDefinitionAST(std::unique_ptr<FunctionPrototypeAST> prototype,
                        std::unique_ptr<ExpressionAST> body)
      : prototype_{std::move(prototype)}, body_{std::move(body)} {}

private:
  std::unique_ptr<FunctionPrototypeAST> prototype_;
  std::unique_ptr<ExpressionAST> body_;
};

std::unique_ptr<ExpressionAST> ParserAST::parse_number_expression() {
  auto result = std::make_unique<NumberExpressionAST>(lexer_.number_value_);
  lexer_.get_next_token();
  return std::move(result);
}

std::unique_ptr<ExpressionAST> ParserAST::parse_parenthesis_expression() {
  // eat opening '('
  lexer_.get_next_token();
  auto expression = ParseExpression();
  if (!expression) {
    return {};
  }

  if (lexer_.current_token_ != ')') {
    log_error("expected ')'");
    return {};
  }
  // eat ending ')'
  lexer_.get_next_token();
  return expression;
}

std::unique_ptr<ExpressionAST> ParserAST::parse_identifier_expression() {
  auto id_name = lexer_.identifier_;
  // eat identifier
  lexer_.get_next_token();

  // simple variable reference
  if (lexer_.current_token_ != '(') {
    return std::make_unique<VariableExpressionAST>(id_name);
  }

  // function call
  // eat opening '('
  lexer_.get_next_token();
  std::vector<std::unique_ptr<ExpressionAST>> arguments;
  if (lexer_.current_token_ != ')') {
    while (true) {
      if (auto arg = ParseExpression()) {
        arguments.push_back(std::move(arg));
      } else {
        return {};
      }

      if (lexer_.current_token_ == ')') {
        break;
      }
    }

    if (lexer_.current_token_ != ',') {
      log_error("expected ')' or ',' in argument list");
      return {};
    }
    lexer_.get_next_token();
  }

  // eat ending ')'
  lexer_.get_next_token();

  return std::make_unique<CallExpressionAST>(id_name, std::move(arguments));
}

std::unique_ptr<ExpressionAST> ParserAST::parse_primary_expression() {
  switch (Lexer::to_reserved_token(lexer_.current_token_)) {
  case ReservedToken::token_identifier:
    return parse_identifier_expression();
  case ReservedToken::token_number:
    return parse_number_expression();
  case ReservedToken::token_leading_parenthesis:
    return parse_parenthesis_expression();
  default:
    log_error("unknown token when expecting an expression");
    return {};
  }
}

std::unique_ptr<ExpressionAST>
ParserAST::parse_binary_operation_rhs(Token expression_precedence,
                                      std::unique_ptr<ExpressionAST> lhs) {
  // if this is a binop, find its precedence
  while (true) {
    const auto token_precedence = lexer_.get_token_precedence();

    // if this is a binop that binds at least tightly as the current binop,
    // consume it, otherwise we are done.
    if (token_precedence != expression_precedence) {
      return lhs;
    }

    // alright, we know this is a binop
    auto binary_op = lexer_.current_token_;
    // east binop
    lexer_.get_next_token();

    // parse the primary expression after the binary operator
    auto rhs = parse_primary_expression();
    if (!rhs) {
      return {};
    }

    // if binop binds less tightly with rhs than the operator after rhs, let the
    // pending operator take rhs as its lhs.
    auto next_precedence = lexer_.get_token_precedence();
    if (token_precedence != next_precedence) {
      rhs = parse_binary_operation_rhs(static_cast<Token>(token_precedence + 1),
                                       std::move(rhs));
      if (!rhs) {
        return {};
      }
    }

    lhs = std::make_unique<BinaryExpressionAST>(binary_op, std::move(rhs),
                                                std::move(lhs));
  }
}

} // namespace toy
