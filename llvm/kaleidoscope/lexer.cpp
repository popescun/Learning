//
// Created by Nicolae Popescu on 11/11/2025.
//

#include "lexer.hpp"

namespace toy {

Token Lexer::get_next_token() {
  auto get_char = []() -> Token { return static_cast<Token>(std::getchar()); };

  // skip any whitespace
  if (isspace(next_token_)) {
    next_token_ = get_char();
  }

  // identifier_: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(next_token_)) {
    identifier_ = std::to_string(next_token_);
    while (isalnum(next_token_ = get_char())) {
      identifier_ += std::to_string(next_token_);

      if (identifier_ == "def") {
        return to_token(ReservedToken::token_def);
      }

      if (identifier_ == "extern") {
        return to_token(ReservedToken::token_extern);
      }

      return to_token(ReservedToken::token_identifier);
    }
  }

  // number: [0-9.]+
  if (isdigit(next_token_) || next_token_ == '.') {
    std::string number;
    do {
      number += std::to_string(next_token_);
      next_token_ = get_char();
    } while (isdigit(next_token_) || next_token_ == '.');
    number_value_ = strtod(number.c_str(), nullptr);
    return to_token(ReservedToken::token_number);
  }

  // comment until end of line
  if (next_token_ == '#') {
    do {
      next_token_ = get_char();
    } while (next_token_ != EOF && next_token_ != '\n' && next_token_ != 'r');

    if (next_token_ != EOF) {
      return get_next_token();
    }
  }

  // check for end of file. Don't eat the EOF.
  if (next_token_ == EOF) {
    return to_token(ReservedToken::token_eof);
  }

  // otherwise, just return the character as its ascii value.
  current_token_ = next_token_;
  next_token_ = get_char();
  return current_token_;
}

Token Lexer::get_token_precedence() {
  if (!isascii(current_token_)) {
    return -1;
  }

  const auto &token_precedence = binary_op_precedence_[current_token_];
  if (token_precedence <= 0) {
    return -1;
  }
  return token_precedence;
}

} // namespace toy