//
// Created by Nicolae Popescu on 11/11/2025.
//

#ifndef TOY_AST_HPP
#define TOY_AST_HPP

#include "lexer.hpp"

#include <memory>

/**
 * Abstract Syntax Tree (aka Parse Tree).
 */

namespace toy {

class ExpressionAST;

struct ParserAST {
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
  std::unique_ptr<ExpressionAST> parse_binary_operation_rhs(Token expression_precedence, std::unique_ptr<ExpressionAST> lhs);

  std::unique_ptr<ExpressionAST> ParseExpression() { return {}; }
  Lexer lexer_;
};

} // namespace toy

#endif // TOY_AST_HPP