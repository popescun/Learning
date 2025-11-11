//
// Created by Nicolae Popescu on 11/11/2025.
//

#ifndef TOY_LEXER_HPP
#define TOY_LEXER_HPP

#include <string>

/**
 * The lexer returns tokens [0-255] if it is an unknown character, otherwise one
 * of these for known things.
 */

namespace toy {

using Token = std::int8_t;

enum class ReservedToken : Token {
  token_eof = -1,

  // commands
  token_def = -2,
  token_extern = -3,

  // primary
  token_identifier_ = -4,
  token_number = -5,
  token_unknown = -6
};

struct Lexer {
  /**
   * Get the next character token from standard input.
   *
   * @return next token
   */
  Token get_token() {
    auto get_char = []() -> Token {
      return static_cast<Token>(std::getchar());
    };

    // skip any whitespace
    if (isspace(last_token_)) {
      last_token_ = get_char();
    }

    // identifier_: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(last_token_)) {
      identifier_ = std::to_string(last_token_);
      while (isalnum(last_token_ = get_char())) {
        identifier_ += std::to_string(last_token_);

        if (identifier_ == "def") {
          return to_token(ReservedToken::token_def);
        }

        if (identifier_ == "extern") {
          return to_token(ReservedToken::token_extern);
        }

        return to_token(ReservedToken::token_identifier_);
      }
    }

    // number: [0-9.]+
    if (isdigit(last_token_) || last_token_ == '.') {
      std::string number;
      do {
        number += std::to_string(last_token_);
        last_token_ = get_char();
      } while (isdigit(last_token_) || last_token_ == '.');
      number_value_ = strtod(number.c_str(), nullptr);
      return to_token(ReservedToken::token_number);
    }

    // comment until end of line
    if (last_token_ == '#') {
      do {
        last_token_ = get_char();
      } while (last_token_ != EOF && last_token_ != '\n' && last_token_ != 'r');

      if (last_token_ != EOF) {
        return get_token();
      }
    }

    // check for end of file. Don't eat the EOF.
    if (last_token_ == EOF) {
      return to_token(ReservedToken::token_eof);
    }

    // otherwise, just return the character as its ascii value.
    const auto current_char = last_token_;
    last_token_ = get_char();
    return current_char;
  }

  static Token to_token(ReservedToken token) {
    return static_cast<Token>(token);
  }

  Token last_token_;
  // filled in if token_identifier
  std::string identifier_;
  // Filled in if token_number
  double number_value_;
};

} // namespace toy

#endif // TOY_LEXER_HPP