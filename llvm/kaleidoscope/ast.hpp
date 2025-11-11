//
// Created by Nicolae Popescu on 11/11/2025.
//

#ifndef TOY_AST_HPP
#define TOY_AST_HPP

#include "lexer.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * Abstract Syntax Tree (aka Parse Tree).
 */

namespace toy {
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
  explicit VariableExpressionAST(std::string  name) : name_{std::move(name)} {}

private:
  std::string name_;
};

/**
 * Expression class for a binary operator like "+".
 */
class BinaryExpressionAST final : public ExpressionAST {
public:
  explicit BinaryExpressionAST(Token op, std::unique_ptr<ExpressionAST> left,
                               std::unique_ptr<ExpressionAST> right)
      : operator_{op}, left_{std::move(left)}, right_{std::move(right)} {}

private:
  Token operator_;
  std::unique_ptr<ExpressionAST> left_, right_;
};

  /**
   * Expression class for function calls.
   */
  class CallExpressionAST final : public ExpressionAST {
  public:
    explicit CallExpressionAST(std::string  callee, std::vector<std::unique_ptr<ExpressionAST>> arguments)
        : callee_{std::move(callee)}, arguments_ {std::move(arguments)} {}

  private:
    std::string callee_;
    std::vector<std::unique_ptr<ExpressionAST>> arguments_;
  };

} // namespace toy

#endif // TOY_AST_HPP