//
// Created by Nicolae Popescu on 11/11/2025.
//

#include "ast.hpp"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <string>
#include <utility>
#include <vector>

namespace toy {

using namespace llvm;

namespace {
void log_error(const char *token) { fprintf(stderr, "error: %s\n", token); }

void log_error_prototype(const char *token) { log_error(token); }

ParserAST &parser_ast = ParserAST::instance();
} // namespace

using Token = Token;
/**
 *  Base class for all expression nodes.
 */
class ExpressionAST {
public:
  virtual ~ExpressionAST() = default;
  virtual llvm::Value *codegen() = 0;
};

/**
 * Expression class for numeric literals like "1.0".
 */
class NumberExpressionAST final : public ExpressionAST {
public:
  explicit NumberExpressionAST(double value) : value_{value} {}
  Value *codegen() override {
    return ConstantFP::get(*parser_ast.llvm_context_, APFloat(value_));
  }

private:
  double value_;
};

/**
 * Expression class for referencing a variable, like "a".
 */
class VariableExpressionAST final : public ExpressionAST {
public:
  explicit VariableExpressionAST(std::string name) : name_{std::move(name)} {}
  Value *codegen() override {
    auto *variable = parser_ast.function_parameters_[name_];
    if (!variable) {
      log_error("unknown variable name");
      return {};
    }
    return variable;
  }

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
  Value *codegen() override {
    auto *left = lhs_->codegen();
    auto *right = rhs_->codegen();
    if (!left || !right) {
      return {};
    }
    switch (operator_) {
    case ReservedToken::token_operator_add:
      // Note: last param in `CreateFAdd` is `Twine` type:
      // https://llvm.org/doxygen/classllvm_1_1Twine.html#details It's a faster
      // representation of strings that uses a binary-tree instead of an array.
      // See also: -
      // https://www.geeksforgeeks.org/dsa/ropes-data-structure-fast-string-concatenation/
      //           - https://stl.boost.org/Rope.html
      return parser_ast.llvm_IR_builder_->CreateFAdd(left, right, "addtmp");
    case ReservedToken::token_operator_subtract:
      return parser_ast.llvm_IR_builder_->CreateFSub(left, right, "subtmp");
    case ReservedToken::token_operator_multiply:
      return parser_ast.llvm_IR_builder_->CreateFMul(left, right, "multmp");
    case ReservedToken::token_operator_less:
      // store cmp result in `left`s
      left = parser_ast.llvm_IR_builder_->CreateFCmpULT(left, right, "cmptmp");
      // convert result to double
      return parser_ast.llvm_IR_builder_->CreateUIToFP(
          left, Type::getDoubleTy(*parser_ast.llvm_context_), "booltmp");
    default:
      log_error("invalid binary operator");
      return {};
    }
  }

private:
  ReservedToken operator_;
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
  Value *codegen() override {
    // look up the name in the global module table
    Function *function = parser_ast.llvm_module_->getFunction(callee_);
    if (!function) {
      log_error("unknown function referenced");
      return {};
    }

    // if argument mismatch error
    if (function->arg_size() != arguments_.size()) {
      log_error("incorrect arguments size");
      return {};
    }

    std::vector<Value *> arguments_values;
    for (const auto &arg : arguments_) {
      arguments_values.push_back(arg->codegen());
    }
    if (arguments_values.empty()) {
      return {};
    }
    return parser_ast.llvm_IR_builder_->CreateCall(function, arguments_values,
                                                   "calltmp");
  }

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
  FunctionPrototypeAST(std::string name,
                       std::vector<std::string> arguments_names)
      : name_{std::move(name)}, arguments_names_{std::move(arguments_names)} {}

private:
  std::string name_;
  std::vector<std::string> arguments_names_;
};

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

ParserAST::ParserAST()
    : llvm_context_{std::make_unique<LLVMContext>()},
      llvm_IR_builder_{std::make_unique<IRBuilder<>>(*llvm_context_)},
      llvm_module_{std::make_unique<Module>("global_module", *llvm_context_)} {}

ParserAST &ParserAST::instance() {
  static std::unique_ptr<ParserAST> instance{new ParserAST()};
  return *instance;
}

std::unique_ptr<ExpressionAST> ParserAST::parse_number_expression() {
  auto result = std::make_unique<NumberExpressionAST>(lexer_.number_value_);
  lexer_.next_token();
  return std::move(result);
}

std::unique_ptr<ExpressionAST> ParserAST::parse_parenthesis_expression() {
  // eat opening '('
  lexer_.next_token();
  auto expression = parse_primary_expression();
  if (!expression) {
    return {};
  }

  if (lexer_.current_token_ != ')') {
    log_error("expected ')'");
    return {};
  }
  // eat ending ')'
  lexer_.next_token();
  return expression;
}

std::unique_ptr<ExpressionAST> ParserAST::parse_identifier_expression() {
  auto id_name = lexer_.identifier_;
  // eat identifier
  lexer_.next_token();

  // simple variable reference
  if (lexer_.current_token_ != '(') {
    return std::make_unique<VariableExpressionAST>(id_name);
  }

  // function call
  // eat opening '('
  lexer_.next_token();
  std::vector<std::unique_ptr<ExpressionAST>> arguments;
  if (lexer_.current_token_ != ')') {
    while (true) {
      if (auto argument = parse_primary_expression()) {
        arguments.push_back(std::move(argument));
      } else {
        return {};
      }

      if (lexer_.current_token_ == ')') {
        break;
      }

      if (lexer_.current_token_ != ',') {
        log_error("expected ')' or ',' in argument list");
        return {};
      }
      lexer_.next_token();
    }
  }

  // eat ending ')'
  lexer_.next_token();

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
    const auto token_precedence = lexer_.get_current_token_precedence();

    // if this is a binop that binds at least tightly as the current binop,
    // consume it, otherwise we are done.
    if (token_precedence < expression_precedence) {
      return lhs;
    }

    // alright, we know this is a binop
    auto binary_op = lexer_.current_token_;
    // eat binop
    lexer_.next_token();

    // parse the primary expression after the binary operator
    auto rhs = parse_primary_expression();
    if (!rhs) {
      return {};
    }

    const auto next_precedence = lexer_.get_current_token_precedence();

    // if binop binds less tightly with rhs than the operator after rhs, let the
    // pending operator take rhs as its lhs.
    if (token_precedence < next_precedence) {
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

std::unique_ptr<ExpressionAST> ParserAST::parse_expression() {
  auto lhs = parse_primary_expression();
  if (!lhs) {
    return {};
  }
  return parse_binary_operation_rhs(0, std::move(lhs));
}

std::unique_ptr<FunctionPrototypeAST> ParserAST::parse_function_prototype() {
  if (lexer_.current_token_ !=
      Lexer::to_token(ReservedToken::token_identifier)) {
    log_error_prototype("expected function name in prototype");
    return {};
  }

  auto function_name = lexer_.identifier_;
  lexer_.next_token();

  if (lexer_.current_token_ != '(') {
    log_error_prototype("expected '(' in prototype");
    return {};
  }

  std::vector<std::string> arguments_names;
  while (lexer_.get_next_token() ==
         Lexer::to_token(ReservedToken::token_identifier)) {
    arguments_names.push_back(lexer_.identifier_);
  }
  if (lexer_.current_token_ != ')') {
    log_error_prototype("expected ')' in prototype");
    return {};
  }

  // success
  // eat ')'
  lexer_.next_token();

  return std::make_unique<FunctionPrototypeAST>(function_name,
                                                std::move(arguments_names));
}

std::unique_ptr<FunctionDefinitionAST> ParserAST::parse_function_definition() {
  // eat 'def'
  lexer_.next_token();

  auto function_prototype = parse_function_prototype();
  if (!function_prototype) {
    return {};
  }

  if (auto expression = parse_expression()) {
    return std::make_unique<FunctionDefinitionAST>(
        std::move(function_prototype), std::move(expression));
  }

  return {};
}

std::unique_ptr<FunctionDefinitionAST> ParserAST::parse_top_level_expression() {
  if (auto expression = parse_expression()) {
    // make a function prototype with anonymous name
    auto function_prototype = std::make_unique<FunctionPrototypeAST>(
        "__anon_expr", std::vector<std::string>());
    return std::make_unique<FunctionDefinitionAST>(
        std::move(function_prototype), std::move(expression));
  }
  return {};
}

std::unique_ptr<FunctionPrototypeAST> ParserAST::parse_external() {
  // eat extern
  lexer_.next_token();
  return parse_function_prototype();
}

void ParserAST::handle_function_definition() {
  if (parse_function_definition()) {
    fprintf(stderr, "parsed a function definition\n");
  } else {
    // skip token for error recovery
    lexer_.next_token();
  }
}

void ParserAST::handle_extern() {
  if (parse_external()) {
    fprintf(stderr, "parsed an external function\n");
  } else {
    // skip token for error recovery
    lexer_.next_token();
  }
}

void ParserAST::handle_top_level_expression() {
  if (parse_top_level_expression()) {
    fprintf(stderr, "parsed a top level expression\n");
  } else {
    // skip token for error recovery
    lexer_.next_token();
  }
}

void ParserAST::main_loop() {
  // prime the first token
  fprintf(stderr, "toy> ");
  lexer_.next_token();

  while (true) {
    switch (Lexer::to_reserved_token(lexer_.current_token_)) {
    case ReservedToken::token_eof:
      return;
    // ignore top-level semicolon
    case ReservedToken::token_semicolon:
      fprintf(stderr, "toy> ");
      lexer_.next_token();
      break;
    case ReservedToken::token_function_definition:
      handle_function_definition();
      break;
    case ReservedToken::token_external_function:
      handle_extern();
      break;
    default:
      handle_top_level_expression();
      break;
    }
  }
}

} // namespace toy
