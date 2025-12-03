//
// Created by Nicolae Popescu on 03/12/2025.
//

#include "ast_expressions.hpp"
#include "ir_code_generator.hpp"

#include <utility>
#include <variant>

#include "ast_parser.hpp"

#include <llvm/ADT/APFloat.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace toy {

using namespace llvm;
using namespace llvm::orc;

NumberExpressionAST::NumberExpressionAST(ParserAST &parser_ast, double value)
    : value_{value}, parser_ast_{parser_ast} {}

Value *NumberExpressionAST::generate_IR_code() {
  return std::visit(IRCodeGenerator{parser_ast_}, ExpressionASTVariant{*this});
}

VariableExpressionAST::VariableExpressionAST(ParserAST &parser_ast,
                                             std::string name)
    : name_{std::move(name)}, parser_ast_{parser_ast} {}

Value *VariableExpressionAST::generate_IR_code() {
  return std::visit(IRCodeGenerator{parser_ast_}, ExpressionASTVariant{*this});
}

BinaryExpressionAST::BinaryExpressionAST(ParserAST &parser_ast, Token op,
                                         std::unique_ptr<ExpressionAST> lhs,
                                         std::unique_ptr<ExpressionAST> rhs)
    : operator_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)},
      parser_ast_{parser_ast} {}
Value *BinaryExpressionAST::generate_IR_code() {
  // ExpressionASTVariant variant =
  //     BinaryExpressionAST{parser_ast_, Lexer::to_token(operator_),
  //                         std::move(lhs_), std::move(rhs_)};
  // todo: check if moving to the variant is robust, and does not leave anything
  // dangling?
  return std::visit(IRCodeGenerator{parser_ast_},
                    ExpressionASTVariant{std::move(*this)});
}

CallExpressionAST::CallExpressionAST(
    ParserAST &parser_ast, std::string callee,
    std::vector<std::unique_ptr<ExpressionAST>> arguments)
    : callee_{std::move(callee)}, arguments_{std::move(arguments)},
      parser_ast_{parser_ast} {}

Value *CallExpressionAST::generate_IR_code() {
  // look up the name in the global module table
  Function *function = parser_ast_.get_function(callee_);
  if (!function) {
    parser_ast_.log_error("unknown function referenced");
    return {};
  }

  // if argument mismatch error
  if (function->arg_size() != arguments_.size()) {
    parser_ast_.log_error("incorrect arguments size");
    return {};
  }

  std::vector<Value *> arguments_values;
  for (const auto &arg : arguments_) {
    arguments_values.push_back(arg->generate_IR_code());
  }
  if (arguments_values.empty()) {
    return {};
  }
  return parser_ast_.llvm_IR_builder_->CreateCall(function, arguments_values,
                                                  "calltmp");
}

FunctionPrototypeAST::FunctionPrototypeAST(
    ParserAST &parser_ast, std::string name,
    std::vector<std::string> arguments_names)
    : name_{std::move(name)}, arguments_names_{std::move(arguments_names)},
      parser_ast_{parser_ast} {}

// Note that llvm `Function` is a `Value` sub-class.
Function *FunctionPrototypeAST::generate_IR_code() {
  // make the function type: double(double, double) etc.
  const std::vector<Type *> arguments_types{
      arguments_names_.size(), Type::getDoubleTy(*parser_ast_.llvm_context_)};
  // This doc needs clarification: "Note that Types in LLVM are uniqued just
  // like Constants are, so you don’t “new” a type, you “get” it."
  // https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html#function-code-generation
  FunctionType *function_type = FunctionType::get(
      Type::getDoubleTy(*parser_ast_.llvm_context_), arguments_types, false);
  Function *function =
      Function::Create(function_type, Function::ExternalLinkage, name_,
                       parser_ast_.llvm_module_.get());
  // set names for all arguments
  std::uint32_t idx{0};
  for (auto &arg : function->args()) {
    arg.setName(arguments_names_[idx++]);
  }

  return function;
}

FunctionDefinitionAST::FunctionDefinitionAST(
    ParserAST &parser_ast, std::unique_ptr<FunctionPrototypeAST> prototype,
    std::unique_ptr<ExpressionAST> body)
    : prototype_{std::move(prototype)}, body_{std::move(body)},
      parser_ast_{parser_ast} {}

/**
 * todo: This code does have a bug, though:
 * If the FunctionAST::codegen() method finds an existing IR Function, it does
 * not validate its signature against the definition’s own prototype. This
 * means that an earlier ‘extern’ declaration will take precedence over the
 * function definition’s signature, which can cause codegen to fail, for
 * instance if the function arguments are named differently. There are a
 * number of ways to fix this bug, see what you can come up with! Here is a
 * testcase:
 *     - extern foo(a);     # ok, defines foo.
 *     - def foo(b) b;      # Error: Unknown variable name. (decl using 'a'
 * takes precedence).
 */
Function *FunctionDefinitionAST::generate_IR_code() {
  // first, transfer ownership of the prototype to function prototypes map,
  // but keep a reference to it for use bellow
  const auto &name = prototype_->name_;
  parser_ast_.function_prototypes_[prototype_->name_] = std::move(prototype_);
  // check for existing function from previous `extern` declaration
  // todo: does it regard all functions added to this module, not only
  // `extern`s
  Function *function = parser_ast_.get_function(name);
  if (!function) {
    parser_ast_.log_error("function cannot be redefined");
    return {};
  }

  // create a new basic block(body) to start insertion into
  BasicBlock *basic_block =
      BasicBlock::Create(*parser_ast_.llvm_context_, "entry", function);
  // from doc: This specifies that created instructions should be appended to
  // the end of the specified block.
  parser_ast_.llvm_IR_builder_->SetInsertPoint(basic_block);

  // record the function arguments in the function parameters map.
  // todo: how global `variable_names_` could be moved locally to a
  // todo: function definition?
  parser_ast_.variable_names_.clear();
  for (auto &arg : function->args()) {
    parser_ast_.variable_names_[std::string{arg.getName()}] = &arg;
  }

  Value *body_value = body_->generate_IR_code();
  if (!body_value) {
    parser_ast_.log_error("function body generated IR code failed");
    function->eraseFromParent();
    return {};
  }

  // finish off the function
  Value *ret_value = parser_ast_.llvm_IR_builder_->CreateRet(body_value);
  if (!ret_value) {
    parser_ast_.log_error("function create ret failed");
    function->eraseFromParent();
    return {};
  }

  // validate the generated code, checking for consistency
  if (verifyFunction(*function, &errs())) {
    parser_ast_.log_error("function verification failed");
    function->eraseFromParent();
    return {};
  }

  // run the optimizer on the function
  if (parser_ast_.function_pass_manager_) {
    parser_ast_.function_pass_manager_->run(
        *function, *parser_ast_.function_analysis_manager_);
  }

  return function;
}

IfExpressionAST::IfExpressionAST(ParserAST &parser_ast,
                                 std::unique_ptr<ExpressionAST> _if,
                                 std::unique_ptr<ExpressionAST> _then,
                                 std::unique_ptr<ExpressionAST> _else)
    : parser_ast_{parser_ast}, if_{std::move(_if)}, then_{std::move(_then)},
      else_{std::move(_else)} {}

Value *IfExpressionAST::generate_IR_code() {
  Value *if_value = if_->generate_IR_code();
  if (!if_value) {
    return {};
  }

  // convert `if` condition to a truth value(bool) by comparing non-equal to
  // 0.0
  if_value = parser_ast_.llvm_IR_builder_->CreateFCmpONE(
      if_value, ConstantFP::get(*parser_ast_.llvm_context_, APFloat(0.0)),
      "ifcond");

  // get enclosing function
  Function *parent_function =
      parser_ast_.llvm_IR_builder_->GetInsertBlock()->getParent();
  if (!parent_function) {
    return {};
  }

  BasicBlock *then_block =
      BasicBlock::Create(*parser_ast_.llvm_context_, "then", parent_function);
  BasicBlock *else_block =
      BasicBlock::Create(*parser_ast_.llvm_context_, "else");
  parser_ast_.llvm_IR_builder_->CreateCondBr(if_value, then_block, else_block);

  // codegen `then` value
  parser_ast_.llvm_IR_builder_->SetInsertPoint(then_block);
  Value *then_value = then_->generate_IR_code();
  if (!then_value) {
    return {};
  }

  BasicBlock *merge_block =
      BasicBlock::Create(*parser_ast_.llvm_context_, "ifcont");
  parser_ast_.llvm_IR_builder_->CreateBr(merge_block);

  // codegen of `then` by changing the current block, update `then` block for
  // PHI.
  then_block = parser_ast_.llvm_IR_builder_->GetInsertBlock();

  // codegen `else` block
  parent_function->insert(parent_function->end(), else_block);
  parser_ast_.llvm_IR_builder_->SetInsertPoint(else_block);
  Value *else_value = else_->generate_IR_code();
  if (!else_value) {
    return {};
  }
  parser_ast_.llvm_IR_builder_->CreateBr(merge_block);
  // codegen of `else` can change the current block, update else block for
  // PHI.
  else_block = parser_ast_.llvm_IR_builder_->GetInsertBlock();

  // emit `merge` block
  parent_function->insert(parent_function->end(), merge_block);
  parser_ast_.llvm_IR_builder_->SetInsertPoint(merge_block);
  PHINode *phi_node = parser_ast_.llvm_IR_builder_->CreatePHI(
      Type::getDoubleTy(*parser_ast_.llvm_context_), 2, "iftmp");
  if (!phi_node) {
    return {};
  }

  phi_node->addIncoming(then_value, then_block);
  phi_node->addIncoming(else_value, else_block);

  return phi_node;
}

ForExpressionAST::ForExpressionAST(ParserAST &parser_ast,
                                   std::string variable_name,
                                   std::unique_ptr<ExpressionAST> start,
                                   std::unique_ptr<ExpressionAST> end,
                                   std::unique_ptr<ExpressionAST> step,
                                   std::unique_ptr<ExpressionAST> body)
    : parser_ast_{parser_ast}, variable_name_{std::move(variable_name)},
      start_{std::move(start)}, end_{std::move(end)}, step_{std::move(step)},
      body_{std::move(body)} {}

Value *ForExpressionAST::generate_IR_code() {
  // first, codegen the start code, without variable in scope
  Value *start_value = start_->generate_IR_code();
  if (!start_value) {
    return {};
  }

  // make new basic block for loop header, inserting after block
  Function *parent_function =
      parser_ast_.llvm_IR_builder_->GetInsertBlock()->getParent();
  BasicBlock *pre_header_block = parser_ast_.llvm_IR_builder_->GetInsertBlock();
  BasicBlock *loop_block =
      BasicBlock::Create(*parser_ast_.llvm_context_, "loop", parent_function);

  // insert an explicit fall through from the current block to the loop block
  parser_ast_.llvm_IR_builder_->CreateBr(loop_block);

  // start insertion in loop block
  parser_ast_.llvm_IR_builder_->SetInsertPoint(loop_block);

  // start PHI node with an entry for start
  PHINode *phi_node = parser_ast_.llvm_IR_builder_->CreatePHI(
      Type::getDoubleTy(*parser_ast_.llvm_context_), 2, variable_name_);
  phi_node->addIncoming(start_value, pre_header_block);

  // within loop, the variable is defined equal to the phi node value.
  // If it shadows an existing variable, we have to restore it, so save it
  // now.
  Value *old_value = parser_ast_.variable_names_[variable_name_];
  // we store variable name to be consumed when `body_` code is generated.
  // Afterward, the old_value is restored.
  parser_ast_.variable_names_[variable_name_] = phi_node;

  // emit the body of the loop. This, like any other expression, can change
  // the current block. Note that we ignore the value computed by the body,
  // but don't allow an error.
  if (!body_->generate_IR_code()) {
    return {};
  }

  // emit step value
  Value *step_value{};
  if (step_) {
    step_value = step_->generate_IR_code();
    if (!step_value) {
      return {};
    }
  } else {
    // if not specified, use 1.0
    step_value = ConstantFP::get(*parser_ast_.llvm_context_, APFloat{1.0});
  }

  Value *next_variable =
      parser_ast_.llvm_IR_builder_->CreateFAdd(phi_node, step_value, "nextvar");

  // compute the end condition
  Value *end_condition = end_->generate_IR_code();
  if (!end_condition) {
    return {};
  }

  // convert end condition to a bool by comparing non-equal to 0.0
  end_condition = parser_ast_.llvm_IR_builder_->CreateFCmpONE(
      end_condition, ConstantFP::get(*parser_ast_.llvm_context_, APFloat{0.0}),
      "loopcond");

  // get loop end block
  BasicBlock *loop_end_block = parser_ast_.llvm_IR_builder_->GetInsertBlock();

  // add new entry to the phi node for the back-edge
  // node: in doc this line is inserted bellow, does it matter? A: no
  phi_node->addIncoming(next_variable, loop_end_block);

  // create the block for the loop exit("afterloop"), and insert it
  // note: maybe is better " instead "afterloop"?
  BasicBlock *loop_after_block = BasicBlock::Create(
      *parser_ast_.llvm_context_, "afterloop", parent_function);

  // insert the condition branch into the end of loop end block
  parser_ast_.llvm_IR_builder_->CreateCondBr(end_condition, loop_block,
                                             loop_after_block);

  // any new code will be inserted in loop after block
  parser_ast_.llvm_IR_builder_->SetInsertPoint(loop_after_block);

  // restore the unshadowed variable
  if (old_value) {
    parser_ast_.variable_names_[variable_name_] = phi_node;
  } else {
    parser_ast_.variable_names_.erase(variable_name_);
  }

  // for expression always returns 0.0
  return Constant::getNullValue(Type::getDoubleTy(*parser_ast_.llvm_context_));

  return {};
}

} // namespace toy
