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
  token_unknown = -1,

  // keywords
  token_function_definition = -2,
  token_external_function = -3,
  // primary, standalone or start of an expression
  token_identifier = -4,
  token_number = -5,
  token_if = -6,
  token_leading_parenthesis = '(',
  // punctuators
  token_trailing_parenthesis = ')',
  // reserved characters
  token_whitespace = ' ',
  token_comma = ',',
  token_semicolon = ';',
  // assignment
  token_assignment = '=',
  // binary operations
  token_operator_add = '+',
  token_operator_subtract = '-',
  token_operator_multiply = '*',
  token_operator_less = '<',
  // input
  token_new_line = '\n',
  token_carriage_return = '\r',
  token_comment = '#',
  token_dot = '.',
  token_eof = -8,
  // flow control
  token_then = -8,
  token_else = -9,
  token_for = -10,
  token_in = -11,
  // exit program
  token_exit = -12
};

#define STRINGIFY(s) #s
#define keyword_token(t) constexpr auto keyword_token_##t{STRINGIFY(t)};

// clang-format off
keyword_token(def)
keyword_token(extern)
keyword_token(if)
keyword_token(then)
keyword_token(else);
keyword_token(for);
keyword_token(in);
// clang-format on

/**
 * The lexer returns tokens [0-255] if it is an unknown character, otherwise
 * one of these for known things.
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
  // track where current token position
  std::uint32_t row_{1};
  std::uint32_t col_{0};
};

} // namespace toy

#endif // TOY_LEXER_HPP