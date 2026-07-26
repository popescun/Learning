#include "cache_warming.hpp"
#include "compile_time_dispatch.hpp"
#include "fast_queue_SPMC_test.hpp"
#include "fast_queue_SPSC_test.hpp"

#include <benchmark/benchmark.h>

int main(int argc, char **argv) {
  // cache_warming::test();
  // compile_time_dispatch::test();
  // Register the SPMC broadcast benchmarks (and run their correctness demo) first; the SPSC
  // driver below owns the single benchmark::Initialize/RunSpecifiedBenchmarks pass, which then
  // executes both the SPSC and the SPMC benchmarks (and honours --benchmark_filter across both).
  fast_queue_spsc::test();
  fast_queue_spmc::test();

  // Forward the real command-line args so --benchmark_filter / --benchmark_repetitions etc.
  // actually take effect (previously argv was empty, so the whole suite always ran).
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}