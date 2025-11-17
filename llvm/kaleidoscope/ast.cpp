//
// Created by Nicolae Popescu on 11/11/2025.
//

#include "ast.hpp"
#include "jit.hpp"

#include <llvm/ADT/APFloat.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

#include <string>
#include <utility>
#include <vector>

namespace toy {

using namespace llvm;
using namespace llvm::orc;

namespace {

ExitOnError ExitOnErr;

constexpr auto kAnonymousExpression{"__anon_expr"};

} // namespace

using Token = Token;

class IRCode {
public:
  virtual ~IRCode() = default;
  virtual Value *generate_IR_code() = 0;
};

/**
 *  Base class for all expression nodes.
 */
class ExpressionAST : public IRCode {
public:
  ~ExpressionAST() override = default;
};

/**
 * Expression class for numeric literals like "1.0".
 */
class NumberExpressionAST final : public ExpressionAST {
public:
  explicit NumberExpressionAST(const ParserAST &parser_ast, double value)
      : parser_ast_{parser_ast}, value_{value} {}
  Value *generate_IR_code() override {
    return ConstantFP::get(*parser_ast_.llvm_context_, APFloat(value_));
  }

private:
  double value_;
  const ParserAST &parser_ast_;
};

/**
 * Expression class for referencing a variable, like "a".
 */
class VariableExpressionAST final : public ExpressionAST {
public:
  explicit VariableExpressionAST(ParserAST &parser_ast, std::string name)
      : parser_ast_{parser_ast}, name_{std::move(name)} {}
  Value *generate_IR_code() override {
    auto *variable = parser_ast_.function_arguments_[name_];
    if (!variable) {
      parser_ast_.log_error("unknown variable name");
      return {};
    }
    return variable;
  }

  std::string name_;
  ParserAST &parser_ast_;
};

/**
 * Expression class for a binary operator like "+".
 */
class BinaryExpressionAST final : public ExpressionAST {
public:
  explicit BinaryExpressionAST(const ParserAST &parser_ast, Token op,
                               std::unique_ptr<ExpressionAST> lhs,
                               std::unique_ptr<ExpressionAST> rhs)
      : parser_ast_{parser_ast}, operator_{op}, lhs_{std::move(lhs)},
        rhs_{std::move(rhs)} {}
  Value *generate_IR_code() override {
    auto *left = lhs_->generate_IR_code();
    auto *right = rhs_->generate_IR_code();
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

private:
  ReservedToken operator_;
  std::unique_ptr<ExpressionAST> lhs_, rhs_;
  const ParserAST &parser_ast_;
};

/**
 * Expression class for function calls.
 */
class CallExpressionAST final : public ExpressionAST {
public:
  explicit CallExpressionAST(
      const ParserAST &parser_ast, std::string callee,
      std::vector<std::unique_ptr<ExpressionAST>> arguments)
      : parser_ast_{parser_ast}, callee_{std::move(callee)},
        arguments_{std::move(arguments)} {}
  Value *generate_IR_code() override {
    // look up the name in the global module table
    Function *function = parser_ast_.llvm_module_->getFunction(callee_);
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

private:
  std::string callee_;
  std::vector<std::unique_ptr<ExpressionAST>> arguments_;
  const ParserAST &parser_ast_;
};

/**
 * This class represents the "prototype" for a function, which captures its
 * name, and its argument names (thus implicitly the number of arguments the
 * function takes).
 */
class FunctionPrototypeAST : IRCode {
public:
  FunctionPrototypeAST(const ParserAST &parser_ast, std::string name,
                       std::vector<std::string> arguments_names)
      : parser_ast_{parser_ast}, name_{std::move(name)},
        arguments_names_{std::move(arguments_names)} {}
  // Note that llvm `Function` is a `Value` sub-class.
  Function *generate_IR_code() override {
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

  [[nodiscard]] const std::string &name() const { return name_; }

private:
  std::string name_;
  std::vector<std::string> arguments_names_;
  const ParserAST &parser_ast_;
};

/**
 * This class represents a function definition itself.
 */
class FunctionDefinitionAST : public IRCode {
public:
  FunctionDefinitionAST(ParserAST &parser_ast,
                        std::unique_ptr<FunctionPrototypeAST> prototype,
                        std::unique_ptr<ExpressionAST> body)
      : parser_ast_{parser_ast}, prototype_{std::move(prototype)},
        body_{std::move(body)} {}

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
  Function *generate_IR_code() override {
    // first, check for existing function from previous `extern` declaration
    // todo: does it regard all functions added to this module, not only
    // `extern` ones?
    Function *function =
        parser_ast_.llvm_module_->getFunction(prototype_->name());
    if (function) {
      parser_ast_.log_error("function cannot be redefined");
      return {};
    }
    function = prototype_->generate_IR_code();
    if (!function) {
      parser_ast_.log_error("function cannot generate IR code");
      return {};
    }
    // `empty()` here means there is no function body yet
    if (!function->empty()) {
      parser_ast_.log_error("function generated IR code is empty");
      return {};
    }

    // create a new basic block(body) to start insertion into
    BasicBlock *basic_block =
        BasicBlock::Create(*parser_ast_.llvm_context_, "begin", function);
    // from doc: This specifies that created instructions should be appended to
    // the end of the specified block.
    parser_ast_.llvm_IR_builder_->SetInsertPoint(basic_block);

    // record the function arguments in the function parameters map.
    // todo: how global `function_arguments_` could be moved locally to a
    // function definition?
    parser_ast_.function_arguments_.clear();
    for (auto &arg : function->args()) {
      parser_ast_.function_arguments_[std::string{arg.getName()}] = &arg;
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

    parser_ast_.function_pass_manager_->run(
        *function, *parser_ast_.function_analysis_manager_);

    return function;
  }

private:
  std::unique_ptr<FunctionPrototypeAST> prototype_;
  std::unique_ptr<ExpressionAST> body_;
  ParserAST &parser_ast_;
};

ParserAST::ParserAST(Jit &jit) : jit_{jit} { init(); }

void ParserAST::init() {
  llvm_context_ = std::make_unique<LLVMContext>();
  llvm_IR_builder_ = std::make_unique<IRBuilder<>>(*llvm_context_);
  llvm_module_ = std::make_unique<Module>("ToyJIT", *llvm_context_);
  function_pass_manager_ = std::make_unique<FunctionPassManager>();
  loop_analysis_manager_ = std::make_unique<LoopAnalysisManager>();
  function_analysis_manager_ = std::make_unique<FunctionAnalysisManager>();
  cgscc_analysis_manager_ = std::make_unique<CGSCCAnalysisManager>();
  module_analysis_manager_ = std::make_unique<ModuleAnalysisManager>();
  pass_instrumentation_callbacks_ =
      std::make_unique<PassInstrumentationCallbacks>();
  standard_instrumentations_ =
      std::make_unique<StandardInstrumentations>(*llvm_context_, true);

  llvm_module_->setDataLayout(jit_.data_layout_);

  standard_instrumentations_->registerCallbacks(
      *pass_instrumentation_callbacks_, module_analysis_manager_.get());

  // add transform passes
  function_pass_manager_->addPass(InstCombinePass());
  // reassociate expression
  function_pass_manager_->addPass(ReassociatePass());
  // eliminate common sub-expression
  function_pass_manager_->addPass(GVNPass());
  // simplify the control flow graph (deleting unreachable blocks, etc).
  function_pass_manager_->addPass(SimplifyCFGPass());

  PassBuilder pass_builder;
  pass_builder.registerModuleAnalyses(*module_analysis_manager_);
  pass_builder.registerFunctionAnalyses(*function_analysis_manager_);
  pass_builder.crossRegisterProxies(
      *loop_analysis_manager_, *function_analysis_manager_,
      *cgscc_analysis_manager_, *module_analysis_manager_);
}

std::unique_ptr<ExpressionAST> ParserAST::parse_number_expression() {
  auto result =
      std::make_unique<NumberExpressionAST>(*this, lexer_.number_value_);
  lexer_.next_token();
  return std::move(result);
}

std::unique_ptr<ExpressionAST> ParserAST::parse_parenthesis_expression() {
  // eat opening '('
  lexer_.next_token();
  auto expression = parse_expression();
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
  if (lexer_.current_token_ !=
      Lexer::to_token(ReservedToken::token_leading_parenthesis)) {
    return std::make_unique<VariableExpressionAST>(*this, id_name);
  }

  // function call
  // eat opening '('
  lexer_.next_token();
  std::vector<std::unique_ptr<ExpressionAST>> arguments;
  if (lexer_.current_token_ !=
      Lexer::to_token(ReservedToken::token_trailing_parenthesis)) {
    while (true) {
      if (auto argument = parse_primary_expression()) {
        arguments.push_back(std::move(argument));
      } else {
        return {};
      }

      if (lexer_.current_token_ ==
          Lexer::to_token(ReservedToken::token_trailing_parenthesis)) {
        break;
      }

      if (lexer_.current_token_ !=
          Lexer::to_token(ReservedToken::token_comma)) {
        log_error("expected ')' or ',' in argument list");
        return {};
      }
      lexer_.next_token();
    }
  }

  // eat ending ')'
  lexer_.next_token();

  return std::make_unique<CallExpressionAST>(*this, id_name,
                                             std::move(arguments));
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

    lhs = std::make_unique<BinaryExpressionAST>(*this, binary_op,
                                                std::move(lhs), std::move(rhs));
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

  if (lexer_.current_token_ !=
      Lexer::to_token(ReservedToken::token_leading_parenthesis)) {
    log_error_prototype("expected '(' in prototype");
    return {};
  }

  // eat leading '('
  lexer_.next_token();

  std::vector<std::string> arguments_names;
  if (lexer_.current_token_ !=
      Lexer::to_token(ReservedToken::token_trailing_parenthesis)) {
    while (true) {
      if (auto expression = parse_identifier_expression()) {
        const auto *variable =
            dynamic_cast<VariableExpressionAST *>(expression.get());
        if (!variable) {
          log_error(" ExpressionAST to VariableExpressionAST cast failed");
          return {};
        }
        arguments_names.push_back(variable->name_);
      } else {
        return {};
      }

      if (lexer_.current_token_ ==
          Lexer::to_token(ReservedToken::token_trailing_parenthesis)) {
        break;
      }

      if (lexer_.current_token_ !=
          Lexer::to_token(ReservedToken::token_comma)) {
        log_error("expected ')' or ',' in argument list");
        return {};
      }
      lexer_.next_token();
    }
  }

  // eat ending ')'
  lexer_.next_token();

  return std::make_unique<FunctionPrototypeAST>(*this, function_name,
                                                std::move(arguments_names));
}

std::unique_ptr<FunctionDefinitionAST> ParserAST::parse_function_definition() {
  // eat 'def'
  lexer_.next_token();

  // signature
  auto function_prototype = parse_function_prototype();
  if (!function_prototype) {
    return {};
  }

  // building block
  if (auto expression = parse_expression()) {
    return std::make_unique<FunctionDefinitionAST>(
        *this, std::move(function_prototype), std::move(expression));
  }

  return {};
}

std::unique_ptr<FunctionDefinitionAST> ParserAST::parse_top_level_expression() {
  if (auto expression = parse_expression()) {
    // make a function prototype with anonymous name
    auto function_prototype = std::make_unique<FunctionPrototypeAST>(
        *this, kAnonymousExpression, std::vector<std::string>());
    return std::make_unique<FunctionDefinitionAST>(
        *this, std::move(function_prototype), std::move(expression));
  }
  return {};
}

std::unique_ptr<FunctionPrototypeAST> ParserAST::parse_external() {
  // eat extern
  lexer_.next_token();
  return parse_function_prototype();
}

void ParserAST::handle_function_definition() {
  if (auto function_definition = parse_function_definition()) {
    if (auto *ir_code = function_definition->generate_IR_code()) {
      fprintf(stderr, "parsed a function definition\n");
      ir_code->print(errs());
      fprintf(stderr, "\n");
      ExitOnErr(jit_.addModule(
          ThreadSafeModule(std::move(llvm_module_), std::move(llvm_context_))));
      init();
    }
  }
  // else {
  //   // skip token for error recovery
  //   lexer_.next_token();
  // }
}

void ParserAST::handle_extern() {
  if (auto function_prototype = parse_external()) {
    if (auto *ir_code = function_prototype->generate_IR_code()) {
      fprintf(stderr, "parsed an external function\n");
      ir_code->print(errs());
      fprintf(stderr, "\n");
    }
  } else {
    // skip token for error recovery
    lexer_.next_token();
  }
}

void ParserAST::handle_top_level_expression() {
  // evaluate a top-level expression into anonymous function
  if (const auto function_definition = parse_top_level_expression()) {
    if (auto *ir_code = function_definition->generate_IR_code()) {
      // create a `ResourceTracker` to track JIT'd memory allocated to our
      // anonymous expression -- that way we can free it after executing.
      const auto resource_tracker = jit_.jit_dylib_.createResourceTracker();
      auto thread_safe_module =
          ThreadSafeModule(std::move(llvm_module_), std::move(llvm_context_));
      ExitOnErr(
          jit_.addModule(std::move(thread_safe_module), resource_tracker));
      init();

      // search the Jit for __anon_expr symbol
      const auto expression_symbol =
          ExitOnErr(jit_.lookup(kAnonymousExpression));

      // get the symbol's address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function.
      double (*function_pointer)() = expression_symbol.toPtr<double (*)()>();
      fprintf(stderr, "evaluated to %f\n", function_pointer());

      // delete the anonymous expression module from Jit
      ExitOnErr(resource_tracker->remove());

      // fprintf(stderr, "parsed a top level expression\n");
      // ir_code->print(errs());
      // fprintf(stderr, "\n");
      //
      // // todo: fix, this removes the IR code, so is not printed on `dump()`
      // //  remove the anonymous expression
      // ir_code->eraseFromParent();
    }
  } else {
    // skip token for error recovery
    lexer_.next_token();
  }
}

void ParserAST::log_error(const char *token) const {
  fprintf(stderr, "error: %s at row %d, col %d\n", token, lexer_.row_,
          lexer_.col_);
}

void ParserAST::log_error_prototype(const char *token) const {
  log_error(token);
}

void ParserAST::run() {
  // prime the first token
  fprintf(stderr, "toy> ");
  lexer_.next_token();

  while (true) {
    switch (Lexer::to_reserved_token(lexer_.current_token_)) {
    case ReservedToken::token_eof:
      return;
    // ignore top-level semicolon
    case ReservedToken::token_semicolon:
    case ReservedToken::token_new_line:
      fprintf(stderr, "toy> ");
      lexer_.next_token();
      break;
    case ReservedToken::token_function_definition:
      handle_function_definition();
      break;
    case ReservedToken::token_external_function:
      handle_extern();
      break;
    case ReservedToken::token_exit:
      fprintf(stderr, "dump module\n");
      llvm_module_->dump();
      return;
    default:
      handle_top_level_expression();
      break;
    }
  }
}

} // namespace toy
