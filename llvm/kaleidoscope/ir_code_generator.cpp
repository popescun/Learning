//
// Created by Nicolae Popescu on 03/12/2025.
//

#include "ir_code_generator.hpp"

namespace toy {
using namespace llvm;

IRCodeGenerator::IRCodeGenerator(ParserAST &parser_ast)
    : parser_ast_{parser_ast} {}

Value *
IRCodeGenerator::operator()(const NumberExpressionAST &expression) const {
  return ConstantFP::get(*parser_ast_.llvm_context_,
                         APFloat(expression.value_));
}

Value *
IRCodeGenerator::operator()(const VariableExpressionAST &expression) const {
  auto *variable = parser_ast_.variable_names_[expression.name_];
  if (!variable) {
    parser_ast_.log_error("unknown variable name");
    return {};
  }
  return variable;
}

Value *
IRCodeGenerator::operator()(const BinaryExpressionAST &expression) const {
  auto *left = expression.lhs_->generate_IR_code();
  auto *right = expression.rhs_->generate_IR_code();
  if (!left || !right) {
    return {};
  }
  switch (expression.operator_) {
  case ReservedToken::token_operator_add:
    // Note: last param in `CreateFAdd` is `Twine` type:
    // https://llvm.org/doxygen/classllvm_1_1Twine.html#details It's a faster
    // representation of strings that uses a binary-tree instead of an array.
    // See also: -
    // https://www.geeksforgeeks.org/dsa/ropes-data-structure-fast-string-concatenation/
    //           - https://stl.boost.org/Rope.html
    return parser_ast_.llvm_IR_builder_->CreateFAdd(left, right, "addtmp");
  case ReservedToken::token_operator_subtract:
    return parser_ast_.llvm_IR_builder_->CreateFSub(left, right, "subtmp");
  case ReservedToken::token_operator_multiply:
    return parser_ast_.llvm_IR_builder_->CreateFMul(left, right, "multmp");
  case ReservedToken::token_operator_less:
    // store cmp result in `left`s
    left = parser_ast_.llvm_IR_builder_->CreateFCmpULT(left, right, "cmptmp");
    // convert result to double
    return parser_ast_.llvm_IR_builder_->CreateUIToFP(
        left, Type::getDoubleTy(*parser_ast_.llvm_context_), "booltmp");
  default:
    parser_ast_.log_error("invalid binary operator");
    return {};
  }
}

}; // namespace toy