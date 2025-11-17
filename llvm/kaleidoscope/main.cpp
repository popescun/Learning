#include <iostream>
#include <ostream>

#include "ast.hpp"

int main() {
  toy::ParserAST::instance().run();
  return 0;
}