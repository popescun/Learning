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
  token_def = -2,
  token_extern = -3,

  // primary
  token_identifier = -4,
  token_number = -5,
  token_leading_parenthesis = '(',
  token_unknown = -6
};

/**
 * The lexer returns tokens [0-255] if it is an unknown character, otherwise one
 * of these for known things.
 */
struct Lexer {
  using BinaryOperationPrecedence = std::map<Token, Token>;

  /**
   * Get the next character token from standard input.
   *
   * @return next token
   */
  Token get_next_token();

  Token get_token_precedence();

  static Token to_token(ReservedToken token) {
    return static_cast<Token>(token);
  }

  static ReservedToken to_reserved_token(Token token) {
    return static_cast<ReservedToken>(token);
  }

  Token next_token_{to_token(ReservedToken::token_unknown)};
  Token current_token_{to_token(ReservedToken::token_unknown)};
  // filled in if token_identifier
  std::string identifier_{"none"};
  // Filled in if token_number
  double number_value_{0};
  BinaryOperationPrecedence binary_op_precedence_;
};

} // namespace toy

#endif // TOY_LEXER_HPP