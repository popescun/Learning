#include "ast.hpp"

int main() {
  toy::ParserAST parser;

  // prime the first token
  fprintf(stderr, "toy> ");
  parser.lexer_.get_next_token();

  // run the main interpreter loop now
  parser.main_loop();

  return 0;
}