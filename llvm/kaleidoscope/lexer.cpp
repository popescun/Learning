//
// Created by Nicolae Popescu on 11/11/2025.
//

#include "lexer.hpp"

namespace toy {

void Lexer::next_token() {
  auto get_char = []() -> Token { return static_cast<Token>(std::getchar()); };

  // skip any whitespace
  while (isspace(next_token_)) {
    next_token_ = get_char();
  }

  // identifier_: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(next_token_)) {
    identifier_ = next_token_;
    while (isalnum(next_token_ = get_char())) {
      identifier_ += next_token_;
    }

    if (identifier_ == "def") {
      current_token_ = to_token(ReservedToken::token_function_definition);
      return;
    }

    if (identifier_ == "extern") {
      current_token_ = to_token(ReservedToken::token_external_function);
      return;
    }

    // otherwise is an identifier
    current_token_ = to_token(ReservedToken::token_identifier);
    return;
  }

  // number: [0-9.]+
  if (isdigit(next_token_) || next_token_ == '.') {
    std::string number;
    do {
      number += next_token_;
      next_token_ = get_char();
    } while (isdigit(next_token_) || next_token_ == '.');
    number_value_ = strtod(number.c_str(), nullptr);
    current_token_ = to_token(ReservedToken::token_number);
    return;
  }

  // comment until end of line
  if (next_token_ == '#') {
    do {
      next_token_ = get_char();
    } while (next_token_ != EOF && next_token_ != '\n' && next_token_ != 'r');

    if (next_token_ != EOF) {
      next_token();
      return;
    }
  }

  // check for end of file. Don't eat the EOF.
  if (next_token_ == EOF) {
    current_token_ = to_token(ReservedToken::token_eof);
    return;
  }

  // otherwise, just return the character as its ascii value
  current_token_ = next_token_;
  next_token_ = get_char();
  return;
}

std::int8_t Lexer::get_current_token_precedence() {
  if (!isascii(current_token_)) {
    return -1;
  }

  // make sure it's a predefined binary operation
  const auto &token_precedence = binary_op_precedence_[current_token_];
  if (token_precedence <= 0) {
    return -1;
  }
  return token_precedence;
}

} // namespace toy