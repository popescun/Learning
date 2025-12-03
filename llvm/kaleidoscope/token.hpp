//
// Created by Nicolae Popescu on 03/12/2025.
//

#ifndef TOY_TOKEN_HPP
#define TOY_TOKEN_HPP

#include <cstdint>

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

} // namespace toy

  // clang-format on

#endif // TOY_TOKEN_HPP