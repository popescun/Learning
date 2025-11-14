//
// Created by Nicolae Popescu on 11/11/2025.
//

#ifndef TOY_AST_HPP
#define TOY_AST_HPP

#include "lexer.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <memory>

/**
 * Abstract Syntax Tree (aka Parse Tree).
 */

namespace toy {

class ExpressionAST;
class FunctionPrototypeAST;
class FunctionDefinitionAST;

/**
 * AST parser as single-tone.
 */
struct ParserAST {
  /**
   * Constructor.
   */
  ParserAST();

  /**
   * Get single-tone parecer instance.
   * @return
   */
  static ParserAST &instance();

  /**
   * Parse a number expression with syntax:
   *   numberexpr ::= number
   *
   * @return a `NumberExpressionAST`
   */
  std::unique_ptr<ExpressionAST> parse_number_expression();

  /**
   * Parse a parenthesis expression with syntax:
   *  parenexpr ::= '(' expression ')'
   *
   * @return an ExpressionAST`
   */
  std::unique_ptr<ExpressionAST> parse_parenthesis_expression();

  /**
   * Parse an identifier expression with syntax:
   *  identifierexpr
   *    ::= identifier // simple variable reference
   *    ::= identifier '(' expression* ')' // a function call
   *
   * @return an ExpressionAST`
   */
  std::unique_ptr<ExpressionAST> parse_identifier_expression();

  /**
   * Parse a primary expression.
   *
   * @return an ExpressionAST`
   */
  std::unique_ptr<ExpressionAST> parse_primary_expression();

  /**
   * Parse binary operation right hand side with syntax:
   *  binoprhs ::= ('+' primary)*
   *
   * @return an ExpressionAST`
   */
  std::unique_ptr<ExpressionAST>
  parse_binary_operation_rhs(Token expression_precedence,
                             std::unique_ptr<ExpressionAST> lhs);

  /**
   * Parse an expression with syntax:
   *    expression ::= primary binoprhs
   *
   * @return an ExpressionAST
   */
  std::unique_ptr<ExpressionAST> parse_expression();

  /**
   * Parse a function prototype with syntax:
   *    prototype ::= id '(' id* ')'
   *
   * @return a FunctionPrototypeAST
   */
  std::unique_ptr<FunctionPrototypeAST> parse_function_prototype();

  /**
   * Parse a function definition with syntax:
   *    definition ::= 'def' prototype expression
   *
   * @return a FunctionDefinitionAST
   */
  std::unique_ptr<FunctionDefinitionAST> parse_function_definition();

  /**
   * Parse top level expression with syntax:
   *    toplevelexpr ::= expression
   *
   * @return a FunctionDefinitionAST
   */
  std::unique_ptr<FunctionDefinitionAST> parse_top_level_expression();

  /**
   * Parse external n with syntax:
   *    external ::= 'extern' prototype
   *
   * @return a FunctionPrototypeAST
   */
  std::unique_ptr<FunctionPrototypeAST> parse_external();

  void handle_function_definition();
  void handle_extern();
  void handle_top_level_expression();

  /**
   * Handle input expressions with syntax:
   *  top ::= definition | external | expression | ';'
   */
  void main_loop();

  Lexer lexer_;
  std::unique_ptr<llvm::LLVMContext> llvm_context_;
  std::unique_ptr<llvm::IRBuilder<>> llvm_IR_builder_;
  std::unique_ptr<llvm::Module> llvm_module_;
  std::map<std::string, llvm::Value *> function_parameters_;
};

} // namespace toy

#endif // TOY_AST_HPP