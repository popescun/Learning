//
// Created by Nicolae Popescu on 28/10/2025.
//
// Tests and benchmarks for fast_queue_SPMC.hpp (single-producer / N-consumer broadcast).
// Kept in a separate file from the implementation, mirroring the SPSC split. Follows the
// same principle as the SPSC benchmarks: the throughput tests measure the queue's RAW
// read/write speed only (no payload decoding/processing); content correctness is proven in
// the dedicated correctness demo below.
//

#pragma once

#include "fast_queue_SPMC.hpp"

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <print>
#include <span>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

namespace fast_queue_spmc {

// A large ring (1 MiB) so producer and the N consumers decouple and back-pressure stalls
// vanish - throughput then reflects the raw fan-out data-movement cost.
constexpr std::size_t LARGE_QUEUE_SIZE = std::size_t{1} << 20;

// --- Correctness demo: broadcast, zero-copy -----------------------------------------------
// One producer + NC consumers on the small 1 KB ring (forces frequent wraps and exercises
// the min() reuse gate). Each consumer reads EVERY message IN PLACE via try_read_view /
// commit_read and asserts it sees the full stream, in order, with no loss.
inline void test_broadcast_zero_copy() {
  std::println("--- test_broadcast_zero_copy ---");
  constexpr std::size_t NC = 3;
  constexpr std::uint64_t N = 1'000'000;
  auto fq_ptr = std::make_unique<spmc_queue_t<QUEUE_SIZE, NC>>();
  auto &fq = *fq_ptr;
  producer prod;
  std::atomic<bool> go{false};
  std::array<std::uint64_t, NC> received{}; // written by each consumer, read after join

  std::vector<std::thread> consumers;
  consumers.reserve(NC);
  for (std::size_t c = 0; c < NC; ++c) {
    consumers.emplace_back([&, c] {
      consumer cons{c};
      std::array<std::byte, 64> scratch{}; // reassemble a wrapped payload for validation only
      std::uint64_t expected = 0;
      while (!go.load(std::memory_order_acquire)) {
        spin_pause();
      }
      while (expected < N) {
        auto view = cons.try_read_view(fq);
        if (!view) {
          spin_pause();
          continue;
        }
        std::memcpy(scratch.data(), view->first.data(), view->first.size());
        if (view->wrapped()) {
          std::memcpy(scratch.data() + view->first.size(), view->second.data(),
                      view->second.size());
        }
        std::uint64_t seq{};
        std::memcpy(&seq, scratch.data(), sizeof(seq));
        assert(seq == expected && "broadcast zero-copy: out of order or lost message");
        cons.commit_read(fq);
        ++expected;
      }
      received[c] = expected;
    });
  }

  std::thread producer_thread([&] {
    while (!go.load(std::memory_order_acquire)) {
      spin_pause();
    }
    for (std::uint64_t seq = 0; seq < N; ++seq) {
      std::array<std::byte, sizeof(seq)> bytes{};
      std::memcpy(bytes.data(), &seq, sizeof(seq));
      while (!prod.try_write(fq, std::span<const std::byte>{bytes})) {
        spin_pause();
      }
    }
  });

  go.store(true, std::memory_order_release);
  producer_thread.join();
  for (auto &t : consumers) {
    t.join();
  }
  for (std::size_t c = 0; c < NC; ++c) {
    assert(received[c] == N && "a consumer did not receive every message");
  }
  std::println("test_broadcast_zero_copy PASSED ({} consumers x {} messages, in order, no loss)", NC,
               N);
}

// --- Broadcast throughput benchmark -------------------------------------------------------
// One producer fans N messages out to NC consumers (each reads all N). We measure the queue's
// raw read/write speed only - the consumer just reads (copy or zero-copy), no decode/process.
// ZeroCopy selects try_read_view/commit_read vs try_read; BusySpin selects the wait strategy.
// Manual timing brackets only the pump (spawn/join and payload build excluded).
template <class Queue, bool BusySpin, bool ZeroCopy>
inline void run_broadcast(benchmark::State &state) {
  const auto N = static_cast<std::uint64_t>(state.range(0));
  constexpr std::size_t NC = Queue::N;
  constexpr std::size_t MAX_MSG = 64;

  auto pause = [] {
    if constexpr (BusySpin) {
      spin_pause();
    } else {
      std::this_thread::yield();
    }
  };

  // Power-of-two pool of pre-built payloads (8..44 bytes), built once outside the timed region
  // so allocation never lands on the hot path and memory is O(POOL), not O(N).
  constexpr std::uint64_t POOL = 8192;
  static_assert((POOL & (POOL - 1)) == 0, "POOL must be a power of two");
  constexpr std::uint64_t POOL_MASK = POOL - 1;
  std::vector<std::vector<std::byte>> pool;
  pool.reserve(POOL);
  for (std::uint64_t j = 0; j < POOL; ++j) {
    pool.emplace_back(sizeof(std::uint64_t) + static_cast<std::size_t>(j % 37));
  }

  std::uint64_t last_fulls = 0;

  for (auto _ : state) {
    auto fq_ptr = std::make_unique<Queue>();
    Queue &fq = *fq_ptr;
    producer prod;
    std::atomic<bool> go{false};
    std::atomic<std::uint64_t> full_events{0};
    std::atomic<std::size_t> done{0};
    // Stamped by the LAST consumer to finish, before any join, so teardown is excluded.
    std::chrono::steady_clock::time_point t_end;

    std::vector<std::thread> consumers;
    consumers.reserve(NC);
    for (std::size_t c = 0; c < NC; ++c) {
      consumers.emplace_back([&, c] {
        consumer cons{c};
        std::array<std::byte, MAX_MSG> out{}; // used by the copy path only
        std::uint64_t got = 0;
        while (!go.load(std::memory_order_acquire)) {
          pause();
        }
        while (got < N) {
          if constexpr (ZeroCopy) {
            auto view = cons.try_read_view(fq);
            if (!view) {
              pause();
              continue;
            }
            benchmark::DoNotOptimize(view->first);
            benchmark::DoNotOptimize(view->second);
            cons.commit_read(fq);
            ++got;
          } else {
            auto n = cons.try_read(fq, out);
            if (!n) {
              pause();
              continue;
            }
            benchmark::DoNotOptimize(out);
            ++got;
          }
        }
        if (done.fetch_add(1, std::memory_order_acq_rel) + 1 == NC) {
          t_end = std::chrono::steady_clock::now(); // last consumer stamps the end
        }
      });
    }

    std::thread producer_thread([&] {
      while (!go.load(std::memory_order_acquire)) {
        pause();
      }
      std::uint64_t fulls = 0;
      for (std::uint64_t seq = 0; seq < N; ++seq) {
        std::span<const std::byte> span{pool[seq & POOL_MASK]};
        while (!prod.try_write(fq, span)) { // slowest consumer gates reuse (lossless)
          ++fulls;
          pause();
        }
      }
      full_events.store(fulls, std::memory_order_relaxed);
    });

    const auto t_begin = std::chrono::steady_clock::now();
    go.store(true, std::memory_order_release);
    producer_thread.join();
    for (auto &t : consumers) {
      t.join();
    }

    state.SetIterationTime(std::chrono::duration<double>(t_end - t_begin).count());
    last_fulls = full_events.load(std::memory_order_relaxed);
  }

  // Items = messages BROADCAST (the producer's fan-out rate); each is delivered to all NC.
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(N));
  std::println("broadcast {} msgs/iteration to {} consumers (read/write speed only); producer hit "
               "a full queue {} times on the last iteration",
               N, NC, last_fulls);
}

// Large ring, busy-spin, 3 consumers - copy read path.
inline void test_broadcast_optimized(benchmark::State &state) {
  std::println("--- test_broadcast_optimized ---");
  run_broadcast<spmc_queue_t<LARGE_QUEUE_SIZE, 3>, /*BusySpin=*/true, /*ZeroCopy=*/false>(state);
  std::println("test_broadcast_optimized PASSED");
}

// Same, but each consumer reads IN PLACE (zero-copy). Head-to-head with the copy variant this
// shows the copy the zero-copy path avoids - and in broadcast that copy is paid PER CONSUMER.
inline void test_broadcast_optimized_zero_copy(benchmark::State &state) {
  std::println("--- test_broadcast_optimized_zero_copy ---");
  run_broadcast<spmc_queue_t<LARGE_QUEUE_SIZE, 3>, /*BusySpin=*/true, /*ZeroCopy=*/true>(state);
  std::println("test_broadcast_optimized_zero_copy PASSED");
}

// Runs the correctness demo and registers the broadcast benchmarks. The actual
// benchmark::Initialize/RunSpecifiedBenchmarks/Shutdown is driven once by fast_queue::test()
// (called after this in main), so these benchmarks run in the same pass as the SPSC ones.
inline void register_benchmarks() {
  test_broadcast_zero_copy();
  // Arg(N) = messages broadcast per iteration. Add more ->Arg()s to sweep N.
  BENCHMARK(test_broadcast_optimized)->UseManualTime()->Iterations(1)->Arg(100'000'000);
  BENCHMARK(test_broadcast_optimized_zero_copy)->UseManualTime()->Iterations(1)->Arg(100'000'000);
}

} // namespace fast_queue_spmc
