#include "cache_warming.hpp"
#include "compile_time_dispatch.hpp"
#include "fast_queue_SPMC_test.hpp"
#include "fast_queue_SPSC_test.hpp"

int main(int argc, char **argv) {
  // cache_warming::test();
  // compile_time_dispatch::test();
  // Register the SPMC broadcast benchmarks (and run their correctness demo) first; the SPSC
  // driver below owns the single benchmark::Initialize/RunSpecifiedBenchmarks pass, which then
  // executes both the SPSC and the SPMC benchmarks (and honours --benchmark_filter across both).
  fast_queue_spmc::register_benchmarks();
  fast_queue::test(argc, argv);
  return 0;
}