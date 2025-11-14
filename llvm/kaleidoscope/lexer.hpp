//
// Created by Nicolae Popescu on 11/11/2025.
//

#ifndef TOY_LEXER_HPP
#define TOY_LEXER_HPP

#include "lexer.hpp"

#include <map>
#include <string>

namespace toy {

using Token = std::int8_t;

enum class ReservedToken : Token {
  token_eof = -1,

  // commands
  token_function_definition = -2,
  token_external_function = -3,

  // primary
  token_identifier = -4,
  token_number = -5,
  token_whitespace = ' ',
  token_leading_parenthesis = '(',
  token_semicolon = ';',
  token_operator_add = '+',
  token_operator_subtract = '-',
  token_operator_multiply = '*',
  token_operator_less = '<',
  token_exit = -6,
  token_unknown = -7
};

/**
 * The lexer returns tokens [0-255] if it is an unknown character, otherwise one
 * of these for known things.
 */
struct Lexer {
  /**
   * Update current token and next character token from standard input.
   *
   * Reads another token, parse it and updates `current_token_` with its
   * results. It also updates next character token `next_token_` without parsing
   * it.
   *
   *
   * @return current token
   */
  void next_token();

  /**
   * Helper function to get next token.
   *
   * @return Token
   */
  Token get_next_token() {
    next_token();
    return current_token_;
  }

  /**
   * Get the precedence of the pending binary operator token.
   * @return token precedence
   */
  std::int8_t get_current_token_precedence();

  // conversion helper
  static Token to_token(ReservedToken token) {
    return static_cast<Token>(token);
  }

  // conversion helper
  static ReservedToken to_reserved_token(Token token) {
    return static_cast<ReservedToken>(token);
  }

  // next character(unparsed) token
  Token next_token_{to_token(ReservedToken::token_whitespace)};
  // current parsed token the lexer is looking at
  Token current_token_{to_token(ReservedToken::token_unknown)};
  // filled in if token_identifier
  std::string identifier_{"none"};
  // filled in if token_number
  double number_value_{0};
  using BinaryOperationPrecedence = std::map<Token, Token>;
  // this holds the precedence for each binary operator token that is defined.
  // Install standard binary operators, first has the lowest precedence.
  BinaryOperationPrecedence binary_op_precedence_ = {
      {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}};
};

} // namespace toy

#endif // TOY_LEXER_HPP