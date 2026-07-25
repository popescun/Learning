#include "cache_warming.hpp"
#include "compile_time_dispatch.hpp"
#include "fast_queue_SPSC_test.hpp"

int main(int argc, char **argv) {
  // cache_warming::test();
  // compile_time_dispatch::test();
  fast_queue::test(argc, argv);
  return 0;
}