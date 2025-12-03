//
// Created by Nicolae Popescu on 03/12/2025.
//

#include "ir_code_generator.hpp"

namespace toy {
using namespace llvm;

IRCodeGenerator::IRCodeGenerator(const ParserAST &parser_ast)
    : parser_ast_{parser_ast} {}

Value *
IRCodeGenerator::operator()(const NumberExpressionAST &expression) const {
  return ConstantFP::get(*parser_ast_.llvm_context_,
                         APFloat(expression.value_));
}
}; // namespace toy