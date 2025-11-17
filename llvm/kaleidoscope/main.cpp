#include <iostream>
#include <ostream>

#include "ast.hpp"

int main() {
  toy::ParserAST::instance().main_loop();
  return 0;
}