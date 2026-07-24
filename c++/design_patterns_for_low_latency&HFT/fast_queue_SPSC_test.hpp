//
// Created by Nicolae Popescu on 28/10/2025.
//
// Tests and benchmarks for fast_queue_SPSC.hpp. Kept out of the implementation header
// so the queue itself reads as a clean, self-contained component. Reopens the
// fast_queue namespace so the demos reference producer/consumer/etc. unqualified,
// exactly as before the split.
//

#pragma once

#include "fast_queue_SPSC.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <print>
#include <span>
#include <thread>
#include <type_traits>
#include <vector>

#include <benchmark/benchmark.h>

namespace fast_queue {

template <class T> std::array<std::byte, sizeof(T)> to_bytes(const T &obj) {
  static_assert(std::is_trivially_copyable_v<T>);
  std::array<std::byte, sizeof(T)> buf{};
  std::memcpy(buf.data(), &obj, sizeof(T)); // memcpy is the well-defined way
  return buf;
}

template <class T> T from_bytes(const std::span<const std::byte> buf) {
  static_assert(std::is_trivially_copyable_v<T>);
  assert(buf.size() >= sizeof(T));
  T obj{};
  std::memcpy(&obj, buf.data(), sizeof(T)); // memcpy is the well-defined way
  return obj;
}

struct alignas(CACHE_LINE_SIZE) Quote { // a POD / trivially-copyable type
  std::uint64_t ts;
  double price;
  std::uint32_t size;
  char symbol[8];
};

// --- Demo 1: single message round-trip -------------------------------------
inline void test_basic() {
  std::println("--- test_basic ---");
  fast_queue fq;
  producer p;
  consumer c;

  Quote q{1'700'000'000ULL, 101.25, 500, "AAPL"};
  const auto bytes = to_bytes(q);
  bool ok = p.try_write(fq, std::span<const std::byte>{bytes});
  std::println("write ok: {}", ok);

  std::array<std::byte, sizeof(Quote)> buf{};
  auto n = c.try_read(fq, buf);
  assert(n && *n == sizeof(Quote));
  const auto q1 = from_bytes<Quote>(buf);
  std::println("q1 ts:{}, size:{}, price:{}, symbol:{}", q1.ts, q1.size, q1.price, q1.symbol);
  assert(!c.try_read(fq, buf) && "queue should be empty now");
}

// --- Demo 2: the limit check --------------------------------------------
// Fill the queue exactly to capacity, confirm the next write is rejected, then
// drain it in order. This exercises "counters checked against the limits".
inline void test_limits() {
  std::println("--- test_limits ---");
  fast_queue fq;
  producer p;
  consumer c;

  constexpr std::size_t rec = sizeof(header_t) + sizeof(std::uint32_t);
  constexpr std::size_t capacity = QUEUE_SIZE / rec; // exact fit

  std::uint32_t v = 0;
  std::size_t written = 0;
  while (true) {
    const auto bytes = to_bytes(v);
    if (!p.try_write(fq, std::span<const std::byte>{bytes})) {
      break; // full - limit reached
    }
    ++v;
    ++written;
  }
  std::println("filled {} messages (capacity {})", written, capacity);
  assert(written == capacity && "should fill exactly up to the limit");

  // One more must be rejected: the counters correctly report the queue is full.
  const auto extra = to_bytes(v);
  assert(!p.try_write(fq, std::span<const std::byte>{extra}) && "queue must report full");

  // Drain everything back out, in order, without loss.
  std::array<std::byte, sizeof(std::uint32_t)> out{};
  for (std::uint32_t expect = 0; expect < written; ++expect) {
    auto n = c.try_read(fq, out);
    assert(n && *n == sizeof(std::uint32_t));
    assert(from_bytes<std::uint32_t>(out) == expect && "out of order / lost");
  }
  assert(!c.try_read(fq, out) && "queue must now be empty");
  std::println("test_limits PASSED");
}

// --- Demo: zero-copy consumer read (in-place, no out-copy) --------------------------------
// Same no-loss / in-order / byte-integrity guarantees as test_full_ring, but the consumer
// reads each message IN PLACE via try_read_view (handling the two-piece wrap case) and then
// releases it with commit_read - exercising the two-phase zero-copy API under real
// back-pressure on the small 1 KB ring (which forces frequent wraps).
inline void test_zero_copy() {
  std::println("--- test_zero_copy ---");
  constexpr std::uint64_t N = 1'000'000;
  auto fq_ptr = std::make_unique<fast_queue>();
  fast_queue &fq = *fq_ptr;
  producer prod;
  consumer cons;
  std::atomic<bool> go{false};

  std::thread producer_thread([&] {
    while (!go.load(std::memory_order_acquire)) {
      spin_pause();
    }
    for (std::uint64_t seq = 0; seq < N; ++seq) {
      const std::size_t extra = static_cast<std::size_t>(seq % 37); // 0..36 -> 8..44 byte payload
      std::array<std::byte, sizeof(std::uint64_t) + 36> buf{};
      std::memcpy(buf.data(), &seq, sizeof(seq));
      for (std::size_t i = 0; i < extra; ++i) {
        buf[sizeof(seq) + i] = static_cast<std::byte>((extra + i) & 0xFF);
      }
      std::span<const std::byte> span{buf.data(), sizeof(seq) + extra};
      while (!prod.try_write(fq, span)) {
        spin_pause();
      }
    }
  });

  std::thread consumer_thread([&] {
    while (!go.load(std::memory_order_acquire)) {
      spin_pause();
    }
    // scratch is used ONLY to reassemble a wrapped payload for validation; real zero-copy
    // consumers would process view.first / view.second directly, without this copy.
    std::array<std::byte, 64> scratch{};
    std::uint64_t expected = 0;
    while (expected < N) {
      auto view = cons.try_read_view(fq);
      if (!view) {
        spin_pause();
        continue;
      }
      const std::size_t sz = view->size();
      assert(sz >= sizeof(std::uint64_t));
      std::memcpy(scratch.data(), view->first.data(), view->first.size());
      if (view->wrapped()) {
        std::memcpy(scratch.data() + view->first.size(), view->second.data(), view->second.size());
      }
      std::uint64_t seq{};
      std::memcpy(&seq, scratch.data(), sizeof(seq));
      assert(seq == expected && "zero-copy: out of order or lost message");

      const std::size_t extra = sz - sizeof(seq);
      for (std::size_t i = 0; i < extra; ++i) {
        assert(scratch[sizeof(seq) + i] == static_cast<std::byte>((extra + i) & 0xFF) &&
               "zero-copy: payload corrupted");
      }
      cons.commit_read(fq); // release the in-place message back to the producer
      ++expected;
    }
    assert(expected == N);
  });

  go.store(true, std::memory_order_release);
  producer_thread.join();
  consumer_thread.join();
  std::println("test_zero_copy PASSED ({} messages read in place, byte-for-byte, no loss)", N);
}

// --- Demo 3: full end-to-end stress -------------------------------------
// Two threads pump N variable-sized messages through a Queue ring. The consumer
// verifies that every message arrives exactly once, in order, byte-for-byte -
// no loss, no duplication, no corruption.
//
// Shared driver for the full-ring benchmarks below. The BusySpin parameter
// selects the wait strategy: true = busy-spin (HFT default, burns the core),
// false = std::this_thread::yield() (cooperative, traps into the scheduler).
// Manual timing brackets only the pump: thread spawn and join are excluded, and
// so is payload construction (built once, up front).
template <class Queue, bool BusySpin>
inline void run_full_ring(benchmark::State &state, std::uint64_t N) {
  // The consumer's scratch buffer is sized to the largest possible message, not
  // to the ring: a 1 MiB ring must never land on the consumer's stack.
  constexpr std::size_t MAX_MSG = 64;

  // The one variable the paired benchmarks compare.
  auto pause = [] {
    if constexpr (BusySpin) {
      spin_pause();
    } else {
      std::this_thread::yield();
    }
  };

  // A small, power-of-two POOL of pre-built payloads the producer cycles through
  // (indexed by seq & POOL_MASK). Building them once, outside the timed region, keeps
  // allocation and filler generation off the hot path - as the old "pre-build all N"
  // approach did - but memory is O(POOL), not O(N), so arbitrarily large N no longer
  // exhausts RAM. It also decouples the producer's payload-source working set from N,
  // so throughput is comparable across message counts (a large-N run no longer streams
  // a bigger source array than a small one). POOL is sized to sit in L2, so the payload
  // source stays cache-hot - this measures the queue path, not payload-fetch cost.
  //
  // Payload length varies with the pool index (8..44 bytes) so records still wrap at
  // unaligned offsets and exercise the split-memcpy path. The filler is a deterministic,
  // seq-INDEPENDENT pattern derived from the payload length, so the consumer can validate
  // byte integrity without knowing which pool slot was used. Per-message identity (order /
  // no-loss) is carried by the true sequence number, which the producer stamps into the
  // first 8 bytes just before each send.
  constexpr std::uint64_t POOL = 8192; // power of two; O(POOL) memory regardless of N
  static_assert((POOL & (POOL - 1)) == 0, "POOL must be a power of two");
  constexpr std::uint64_t POOL_MASK = POOL - 1;

  std::vector<std::vector<std::byte>> pool;
  pool.reserve(POOL);
  for (std::uint64_t j = 0; j < POOL; ++j) {
    const std::size_t extra = static_cast<std::size_t>(j % 37);
    std::vector<std::byte> p(sizeof(std::uint64_t) + extra);
    // First 8 bytes are a placeholder for the per-send sequence number (stamped below).
    for (std::size_t i = 0; i < extra; ++i) {
      p[sizeof(std::uint64_t) + i] = static_cast<std::byte>((extra + i) & 0xFF);
    }
    pool.push_back(std::move(p));
  }

  std::uint64_t last_fulls = 0;

  for (auto _ : state) {
    // Fresh queue per iteration so every iteration pumps exactly N messages.
    // Heap-allocated because a large ring won't fit on the stack; the allocation
    // happens before timing starts, so it is not measured.
    auto fq_ptr = std::make_unique<Queue>();
    Queue &fq = *fq_ptr;
    producer prod;
    consumer cons;

    // Start-gate: spawn both threads first, then release them together. This
    // lets us start the clock *after* thread creation so spawn cost is excluded.
    std::atomic<bool> go{false};
    std::atomic<std::uint64_t> full_events{0};
    // Stamped by the consumer the instant it delivers the last message - before
    // any join - so thread teardown is excluded from the measurement too.
    std::chrono::steady_clock::time_point t_end;

    std::thread producer_thread([&] {
      while (!go.load(std::memory_order_acquire)) {
        pause(); // wait at the gate
      }
      std::uint64_t fulls = 0;
      for (std::uint64_t seq = 0; seq < N; ++seq) {
        // Reuse a pooled buffer and stamp the true sequence number into its first
        // 8 bytes - the only per-message write on the hot path (~one 8-byte store,
        // like a real feed stamping a seq). try_write copies the bytes into the ring
        // before returning, so re-stamping the shared buffer next iteration is safe.
        auto &buf = pool[seq & POOL_MASK];
        std::memcpy(buf.data(), &seq, sizeof(seq));
        std::span<const std::byte> span{buf};
        // Back-pressure: busy-spin until there is room. This is what drives the
        // queue to full without ever dropping a message.
        while (!prod.try_write(fq, span)) {
          ++fulls;
          pause();
        }
      }
      full_events.store(fulls, std::memory_order_relaxed);
    });

    std::thread consumer_thread([&] {
      std::array<std::byte, MAX_MSG> out{};
      std::uint64_t expected = 0;
      while (!go.load(std::memory_order_acquire)) {
        pause(); // wait at the gate
      }
      while (expected < N) {
        auto n = cons.try_read(fq, out);
        if (!n) {
          pause();
          continue;
        }
        std::uint64_t seq{};
        assert(*n >= sizeof(seq));
        std::memcpy(&seq, out.data(), sizeof(seq));
        assert(seq == expected && "out of order or lost message");

        const std::size_t extra = *n - sizeof(seq);
        for (std::size_t i = 0; i < extra; ++i) {
          assert(out[sizeof(seq) + i] == static_cast<std::byte>((extra + i) & 0xFF) &&
                 "payload corrupted");
        }
        ++expected;
      }
      assert(expected == N);
      t_end = std::chrono::steady_clock::now();
    });

    // Threads exist and are parked at the gate. Start timing, then release them.
    const auto t_begin = std::chrono::steady_clock::now();
    go.store(true, std::memory_order_release);

    producer_thread.join();
    consumer_thread.join();
    // t_end was captured just before these joins, so join/teardown is excluded.
    // The joins also establish happens-before, so reading t_end here is safe.

    state.SetIterationTime(std::chrono::duration<double>(t_end - t_begin).count());
    last_fulls = full_events.load(std::memory_order_relaxed);
  }

  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(N));

  std::println("delivered {} messages/iteration in order, byte-for-byte, no loss", N);
  std::println("producer hit a full queue {} times on the last iteration", last_fulls);
}

// Small ring: producer and consumer collide constantly, so the ring is full or
// empty most of the time. This isolates the cost of contention / back-pressure.
inline void test_full_ring_back_pressure(benchmark::State &state) {
  std::println("--- test_full_ring_back_pressure ---");
  run_full_ring<fast_queue_t<QUEUE_SIZE>, /*BusySpin=*/true>(state, 1'000'000'000);
  std::println("test_full_ring_back_pressure PASSED");
}

// Same small ring, but waits with std::this_thread::yield() instead of a
// busy-spin - the original strategy. Compare against the busy-spin version to
// see the cost of trapping into the scheduler under heavy back-pressure.
inline void test_full_ring_back_pressure_yield(benchmark::State &state) {
  std::println("--- test_full_ring_back_pressure_yield ---");
  run_full_ring<fast_queue_t<QUEUE_SIZE>, /*BusySpin=*/false>(state, 1'000'000'000);
  std::println("test_full_ring_back_pressure_yield PASSED");
}

// Large ring: producer and consumer decouple and almost never hit full/empty,
// so the back-pressure stalls vanish and throughput reflects the queue's raw
// data-movement cost.
inline void test_full_ring_optimized(benchmark::State &state) {
  std::println("--- test_full_ring_optimized ---");
  run_full_ring<fast_queue_t<LARGE_QUEUE_SIZE>, /*BusySpin=*/true>(state, 1'000'000'000);
  std::println("test_full_ring_optimized PASSED");
}

// Same large ring, waiting with std::this_thread::yield(). With almost no
// full/empty stalls the wait strategy rarely fires, so this should sit close to
// the busy-spin version - isolating how much the yield cost depends on contention.
inline void test_full_ring_optimized_yield(benchmark::State &state) {
  std::println("--- test_full_ring_optimized_yield ---");
  run_full_ring<fast_queue_t<LARGE_QUEUE_SIZE>, /*BusySpin=*/false>(state, 100'000'000);
  std::println("test_full_ring_optimized_yield PASSED");
}

// --- Demo 4: end-to-end latency under a realistic arrival rate -----------
// Models an HFT feed: messages arrive at a target rate (msgs/sec) with real gaps
// between them - nobody sleeps. The producer BUSY-WAITS on the clock until the
// next scheduled send (like a feed handler busy-polling the NIC), and the
// consumer busy-polls the queue. Each message carries its publish timestamp, so
// the consumer measures true end-to-end delivery latency. The BusySpin flag
// selects the consumer's queue-wait strategy (busy-spin vs yield) so we can see
// how much a descheduled consumer costs during the idle gaps between messages.
struct latency_msg { // trivially copyable so to_bytes/from_bytes work
  std::uint64_t seq;
  std::int64_t t_send_ns;
};

template <bool BusySpin>
inline void run_latency(benchmark::State &state) {
  const auto rate = static_cast<std::uint64_t>(state.range(0)); // messages / second
  constexpr std::uint64_t N = 50'000;                           // samples per iteration
  constexpr std::size_t MAX_MSG = 64;

  using clock = std::chrono::steady_clock;
  const auto period = std::chrono::nanoseconds(
      rate == 0 ? 0 : static_cast<std::int64_t>(1'000'000'000ULL / rate));
  auto now_ns = [] {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now().time_since_epoch())
        .count();
  };

  std::int64_t sum_ns = 0;
  std::int64_t min_ns = std::numeric_limits<std::int64_t>::max();
  std::int64_t max_ns = 0;
  std::uint64_t samples = 0;

  // Every per-message latency, kept so we can sort for tail percentiles after the run.
  // The mean hides the tail and `max` is a single noisy sample; p99/p99.9 are what
  // actually characterize HFT latency (you get picked off on your worst cases).
  std::vector<std::int64_t> all_lat;
  all_lat.reserve(static_cast<std::size_t>(N) * static_cast<std::size_t>(state.max_iterations));

  // NOTE: deliberately NO payload pool here (unlike run_full_ring). This benchmark measures
  // end-to-end DELIVERY LATENCY, not throughput. It sends a modest, fixed N of messages at a
  // target arrival rate and records each message's transit time (recv - t_send). Two reasons
  // a pool buys nothing:
  //   1. N is small - latency percentiles stabilise well below a million samples, so there is
  //      no huge count to keep allocation off the hot path for (the pool in run_full_ring only
  //      exists to survive billion-message throughput runs).
  //   2. Every message must carry a FRESH send timestamp stamped as late as possible, so there
  //      is nothing static to pre-build or reuse - the whole message is rebuilt each send.
  // So each message is just built on the stack (to_bytes) and sent. Simpler and clearer for a
  // latency test.

  for (auto _ : state) {
    auto fq_ptr = std::make_unique<fast_queue>();
    fast_queue &fq = *fq_ptr;
    producer prod;
    consumer cons;
    std::atomic<bool> go{false};

    // Consumer-side accumulators, published to the loop after the join.
    std::int64_t c_sum = 0;
    std::int64_t c_min = std::numeric_limits<std::int64_t>::max();
    std::int64_t c_max = 0;
    std::vector<std::int64_t> c_lat;
    c_lat.reserve(N);

    std::thread consumer_thread([&] {
      std::array<std::byte, MAX_MSG> out{};
      std::uint64_t got = 0;
      while (!go.load(std::memory_order_acquire)) {
        spin_pause();
      }
      while (got < N) {
        auto n = cons.try_read(fq, out);
        if (!n) {
          if constexpr (BusySpin) {
            spin_pause(); // stay hot: notice the next message in ~tens of ns
          } else {
            std::this_thread::yield(); // give up the core: risks a reschedule delay
          }
          continue;
        }
        const std::int64_t recv = now_ns(); // stamp arrival as early as possible
        const auto m = from_bytes<latency_msg>(std::span<const std::byte>{out.data(), *n});
        const std::int64_t lat = recv - m.t_send_ns;
        c_sum += lat;
        c_min = std::min(c_min, lat);
        c_max = std::max(c_max, lat);
        c_lat.push_back(lat);
        ++got;
      }
    });

    std::thread producer_thread([&] {
      while (!go.load(std::memory_order_acquire)) {
        spin_pause();
      }
      const auto t0 = clock::now();
      for (std::uint64_t seq = 0; seq < N; ++seq) {
        // Busy-wait (never sleep) until this message's scheduled send time.
        const auto target = t0 + period * static_cast<std::int64_t>(seq);
        while (clock::now() < target) {
          spin_pause();
        }
        const latency_msg m{seq, now_ns()}; // stamp send as late as possible
        const auto bytes = to_bytes(m);
        while (!prod.try_write(fq, std::span<const std::byte>{bytes})) {
          spin_pause();
        }
      }
    });

    go.store(true, std::memory_order_release);
    producer_thread.join();
    consumer_thread.join();

    sum_ns += c_sum;
    min_ns = std::min(min_ns, c_min);
    max_ns = std::max(max_ns, c_max);
    all_lat.insert(all_lat.end(), c_lat.begin(), c_lat.end());
    samples += N;
  }

  const double avg_ns =
      samples ? static_cast<double>(sum_ns) / static_cast<double>(samples) : 0.0;

  // Nearest-rank percentiles over the sorted samples: p = value at index ceil(q*n)-1.
  std::sort(all_lat.begin(), all_lat.end());
  auto pct = [&all_lat](double q) -> std::int64_t {
    if (all_lat.empty()) {
      return 0;
    }
    auto idx = static_cast<std::size_t>(q * static_cast<double>(all_lat.size()));
    if (idx >= all_lat.size()) {
      idx = all_lat.size() - 1;
    }
    return all_lat[idx];
  };
  const std::int64_t p50 = pct(0.50);
  const std::int64_t p99 = pct(0.99);
  const std::int64_t p999 = pct(0.999);

  state.counters["avg_ns"] = avg_ns;
  state.counters["min_ns"] = static_cast<double>(min_ns);
  state.counters["p50_ns"] = static_cast<double>(p50);
  state.counters["p99_ns"] = static_cast<double>(p99);
  state.counters["p99.9_ns"] = static_cast<double>(p999);
  state.counters["max_ns"] = static_cast<double>(max_ns);

  std::println("rate {} msg/s | avg {:.0f} ns | min {} ns | p50 {} ns | p99 {} ns | p99.9 {} ns | "
               "max {} ns | samples {}",
               rate, avg_ns, min_ns, p50, p99, p999, max_ns, samples);
}

// Consumer busy-spins on an empty queue - the HFT production strategy.
inline void test_latency(benchmark::State &state) {
  std::println("--- test_latency (busy-spin) ---");
  run_latency</*BusySpin=*/true>(state);
}

// Consumer yields on an empty queue - watch the tail (max) blow up as the gaps
// let the OS deschedule it between messages.
inline void test_latency_yield(benchmark::State &state) {
  std::println("--- test_latency (yield) ---");
  run_latency</*BusySpin=*/false>(state);
}

inline void test() {
  test_basic();
  test_limits();
  test_zero_copy();
  BENCHMARK(test_full_ring_back_pressure)->UseManualTime()->Iterations(1);
  BENCHMARK(test_full_ring_back_pressure_yield)->UseManualTime()->Iterations(1);
  BENCHMARK(test_full_ring_optimized)->UseManualTime()->Iterations(1);
  BENCHMARK(test_full_ring_optimized_yield)->UseManualTime()->Iterations(1);
  // Sweep a couple of representative arrival rates (msgs/sec).
  BENCHMARK(test_latency)->UseRealTime()->Iterations(1)->Arg(100'000)->Arg(1'000'000'000);
  BENCHMARK(test_latency_yield)->UseRealTime()->Iterations(1)->Arg(100'000)->Arg(1'000'000'000);

  int argc{0};
  char **argv{nullptr};
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
}
} // namespace fast_queue
