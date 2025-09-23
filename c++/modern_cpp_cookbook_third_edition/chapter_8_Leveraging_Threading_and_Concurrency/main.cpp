#include "threading_basic.hpp"
#include "using_joinable_threads.hpp"

int main() {
  threading_basic::test();
  using_joinable_threads::test();
  return 0;
}