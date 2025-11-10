//
// Created by Nicolae Popescu on 28/10/2025.
//

#pragma once

#include <algorithm>
#include <benchmark/benchmark.h>
#include <iomanip>
#include <iostream>
#include <vector>

#include "utils.hpp"

namespace cache_warming {

constexpr int kSize = 10000000;
std::vector<int> data(kSize);
std::vector<int> indices(kSize);

utils::measure_time measure_cold;
utils::measure_time measure_warm;

inline void BM_CacheCold(benchmark::State &state) {
  // Generate random indices
  for (auto &index : indices) {
    index = rand() % kSize;
  }

  for (auto s : state) {
    measure_cold.start();
    int sum = 0;
    // Access data in random order
    for (int i = 0; i < kSize; ++i) {
      benchmark::DoNotOptimize(sum += data[indices[i]]);
    }
    measure_cold.stop();
    benchmark::ClobberMemory();
  }
}

inline void BM_CacheWarm(benchmark::State &state) {
  // Warm cache by accessing data in sequential order
  int sum_warm = 0;
  for (int i = 0; i < kSize; ++i) {
    benchmark::DoNotOptimize(sum_warm += data[i]);
  }
  benchmark::ClobberMemory();

  // Run the benchmark
  for (auto _ : state) {
    measure_warm.start();
    int sum = 0;
    // Access data in sequential order again
    for (int i = 0; i < kSize; ++i) {
      benchmark::DoNotOptimize(sum += data[i]);
    }
    measure_warm.stop();
    benchmark::ClobberMemory();
  }
}

inline void test() {
  BENCHMARK(BM_CacheCold);
  BENCHMARK(BM_CacheWarm);

  // todo: extract this to an utils that can be used in all `test` methods
  int argc{0};
  char **argv{nullptr};
  // benchmark::MaybeReenterWithoutASLR(argc, argv);
  // char arg0_default[] = "benchmark";
  // char* args_default = reinterpret_cast<char*>(arg0_default);
  // if (!argv) {
  //   argc = 1;
  //   argv = &args_default;
  // }
  ::benchmark::Initialize(&argc, argv);
  // if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return;
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();

  // todo: for some reason the time is not the same as in the google benchmark
  std::cout << std::fixed
            << "BM_CacheCold:" << measure_cold.get_accumulated_time_ns()
            << " ns" << std::endl;
  std::cout << std::fixed
            << "BM_CacheWarm:" << measure_warm.get_accumulated_time_ns()
            << " ns" << std::endl;
}
} // namespace cache_warming
