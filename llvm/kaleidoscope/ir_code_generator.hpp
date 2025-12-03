//
// Created by Nicolae Popescu on 03/12/2025.
//

#ifndef TOY_IR_CODE_GENERATOR_HPP
#define TOY_IR_CODE_GENERATOR_HPP

#include "ast_parser.hpp"

#include <variant>

namespace toy {

using ExpressionASTVariant =  std::variant<NumberExpressionAST,
VariableExpressionAST,
  BinaryExpressionAST, CallExpressionAST, FunctionPrototypeAST,
  FunctionDefinitionAST, IfExpressionAST, ForExpressionAST>;

struct IRCodeGenerator {
  const ParserAST &parser_ast_;

  explicit IRCodeGenerator(const ParserAST &parser_ast);

  llvm::Value *operator()(const NumberExpressionAST &expression) const;
  llvm::Value *operator()(const VariableExpressionAST &expression) const{ return{}; }
  llvm::Value *operator()(const BinaryExpressionAST &expression) const{ return{}; }
  llvm::Value *operator()(const CallExpressionAST &expression) const{ return{}; }
  llvm::Value *operator()(const FunctionPrototypeAST &expression) const{ return{}; }
  llvm::Value *operator()(const FunctionDefinitionAST &expression) const{ return{}; }
  llvm::Value *operator()(const IfExpressionAST &expression) const{ return{}; }
  llvm::Value *operator()(const ForExpressionAST &expression) const{ return{}; }
};

} // namespace toy

#endif // TOY_IR_CODE_GENERATOR_HPP