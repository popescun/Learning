//
// Created by Nicolae Popescu on 28/10/2025.
//

#pragma once

#include <benchmark/benchmark.h>

#include "utils.hpp"

namespace compile_time_dispatch {

class Base {
public:
  virtual ~Base() {}
  virtual int function() const = 0;
};

class Derived1 : public Base {
public:
  int function() const override { return 1; }
};

class Derived2 : public Base {
public:
  int function() const override { return 2; }
};

// Benchmark for runtime dispatch
static void BM_RuntimeDispatch(benchmark::State &state) {
  Base *obj;
  if (state.range(0) == 1)
    obj = new Derived1;
  else
    obj = new Derived2;

  for (auto _ : state) {
    benchmark::DoNotOptimize(obj->function());
  }

  delete obj;
}

// Benchmark for compile-time dispatch
template <typename T>
static void BM_CompileTimeDispatch(benchmark::State &state) {
  T obj;
  for (auto _ : state) {
    benchmark::DoNotOptimize(obj.function());
  }
}

inline void test() {
  BENCHMARK(BM_RuntimeDispatch)->Arg(1)->Arg(2);
  BENCHMARK_TEMPLATE(BM_CompileTimeDispatch, Derived1);
  BENCHMARK_TEMPLATE(BM_CompileTimeDispatch, Derived2);

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
  // std::cout << std::fixed
  //           << "BM_CacheCold:" << measure_cold.get_accumulated_time_ns()
  //           << " ns" << std::endl;
  // std::cout << std::fixed
  //           << "BM_CacheWarm:" << measure_warm.get_accumulated_time_ns()
  //           << " ns" << std::endl;
}
} // namespace compile_time_dispatch
