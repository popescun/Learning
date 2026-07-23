//
// Created by Nicolae Popescu on 28/10/2025.
//

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <print>
#include <span>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#if defined(__cpp_lib_hardware_interference_size)
#include <new>
inline constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#elif defined(__aarch64__) && defined(__APPLE__)
inline constexpr std::size_t CACHE_LINE_SIZE = 128; // Apple Silicon
#else
inline constexpr std::size_t CACHE_LINE_SIZE = 64; // safe default
#endif

namespace fast_queue {

// The ring capacity must be a power of two so a monotonically increasing byte
// counter can be mapped onto a buffer offset with a cheap bit-mask instead of a
// modulo.  Keeping it small makes it easy to drive the queue all the way to
// full and wrap around many times in the tests below.
constexpr std::size_t QUEUE_SIZE = 1024;
constexpr std::uint64_t QUEUE_MASK = QUEUE_SIZE - 1;
static_assert((QUEUE_SIZE & QUEUE_MASK) == 0, "QUEUE_SIZE must be a power of two");

// Each message record is: [int32 length][payload bytes].
using header_t = std::int32_t;

/**
 * Single-producer / single-consumer byte ring buffer.
 *
 * Two monotonically increasing byte counters describe the state:
 *  - write_counter: total bytes committed by the producer (head). Written by
 *    the producer, read by the consumer.
 *  - read_counter:  total bytes consumed by the consumer  (tail). Written by
 *    the consumer, read by the producer.
 *
 * The number of bytes currently available to read = write_counter - read_counter;
 * it is always in the range [0, QUEUE_SIZE].
 *  - empty  <=> write_counter == read_counter
 *  - full   <=> write_counter - read_counter == QUEUE_SIZE
 *
 * Because fullness/emptiness are distinguished by the counter *difference* (not
 * by offset equality), the whole buffer can be used - there is no wasted slot.
 * The physical position of a counter in the buffer is (counter & QUEUE_MASK).
 */
struct fast_queue {
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> read_counter{0};
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  alignas(CACHE_LINE_SIZE) std::array<std::byte, QUEUE_SIZE> buffer{};
};

// Copy n bytes into the ring starting at logical(absolute) counter `counter`, wrapping around the
// physical end of the buffer when necessary (circular write).
inline void ring_write(fast_queue &fq, std::uint64_t counter, const std::byte *src, std::size_t n) {
  const auto index = static_cast<std::size_t>(counter & QUEUE_MASK);
  const std::size_t first = std::min(n, QUEUE_SIZE - index);
  std::memcpy(fq.buffer.data() + index, src, first);
  if (n > first) { // the record straddles the end -> continue at the start
    std::memcpy(fq.buffer.data(), src + first, n - first);
  }
}

// Copy n bytes out of the ring starting at logical(absolute) counter `counter`, wrapping around the
// physical end of the buffer when necessary (circular read).
inline void ring_read(const fast_queue &fq, std::uint64_t counter, std::byte *dst, std::size_t n) {
  const auto index = static_cast<std::size_t>(counter & QUEUE_MASK);
  const std::size_t first = std::min(n, QUEUE_SIZE - index);
  std::memcpy(dst, fq.buffer.data() + index, first);
  if (n > first) {
    std::memcpy(dst + first, fq.buffer.data(), n - first);
  }
}

struct producer {
  /**
   * Try to write one message. Returns false (nothing written) when the queue
   * does not have room for the whole record - this is the limit check that
   * gives us back-pressure and guarantees the consumer never loses data.
   */
  bool try_write(fast_queue &fq, std::span<const std::byte> payload) {
    const auto payload_size = static_cast<header_t>(payload.size());
    const std::size_t record_size = sizeof(header_t) + payload_size;
    assert(record_size <= QUEUE_SIZE && "message larger than the whole queue");

    // Check the free space against the limit. First use the cached tail to
    // avoid touching the consumer's cache line on every call; only refresh from
    // the shared counter if that suggests we might be full.
    std::uint64_t bytes_available_to_read = write_counter - read_counter;
    if (bytes_available_to_read + record_size > QUEUE_SIZE) {
      read_counter = fq.read_counter.load(std::memory_order_acquire);
      bytes_available_to_read = write_counter - read_counter;
      if (bytes_available_to_read + record_size > QUEUE_SIZE) {
        return false; // genuinely full
      }
    }

    // Write the message: payload size prefix followed by the payload. Both copies go
    // through ring_write so a record that reaches the end of the buffer wraps around
    // to the beginning.
    ring_write(fq, write_counter, reinterpret_cast<const std::byte *>(&payload_size),
               sizeof(payload_size));
    ring_write(fq, write_counter + sizeof(payload_size), payload.data(), payload.size());

    write_counter += record_size;
    // Publish: everything up to write_counter is now safe for the consumer to
    // read. release pairs with the consumer's acquire load.
    fq.write_counter.store(write_counter, std::memory_order_release);
    return true;
  }

  std::uint64_t write_counter{0}; // private copy of the head
  std::uint64_t read_counter{0};  // last observed tail (consumer progress)
};

struct consumer {
  /**
   * Try to read one message into `out`. Returns the number of payload bytes
   * read, or std::nullopt when the queue is empty.
   */
  std::optional<std::size_t> try_read(fast_queue &fq, std::span<std::byte> out) {
    // Empty check. Use the cached head first, refresh only when it looks empty.
    if (read_counter == write_counter) {
      write_counter = fq.write_counter.load(std::memory_order_acquire);
      if (read_counter == write_counter) {
        return std::nullopt; // nothing to read
      }
    }

    header_t payload_size{};
    ring_read(fq, read_counter, reinterpret_cast<std::byte *>(&payload_size), sizeof(payload_size));
    assert(payload_size >= 0 && static_cast<std::size_t>(payload_size) <= out.size() &&
           "output buffer isn't large enough for the message");

    ring_read(fq, read_counter + sizeof(payload_size), out.data(),
              static_cast<std::size_t>(payload_size));

    const std::size_t record_size = sizeof(payload_size) + static_cast<std::size_t>(payload_size);
    read_counter += record_size;
    // Publish: the producer may now reuse the space we just consumed.
    fq.read_counter.store(read_counter, std::memory_order_release);
    return static_cast<std::size_t>(payload_size);
  }

  std::uint64_t read_counter{0};  // private copy of the tail
  std::uint64_t write_counter{0}; // last observed head (producer progress)
};

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

// --- Demo 3: full end-to-end stress -------------------------------------
// Two threads pump N variable-sized messages through the small ring. Because
// the ring is tiny relative to N, it fills up and wraps around tens of
// thousands of times. The consumer verifies that every message arrives exactly
// once, in order, byte-for-byte - i.e. no loss, no duplication, no corruption.
inline void test_full_ring(benchmark::State &state) {
  std::println("--- test_full_ring ---");
  constexpr std::uint64_t N = 1'000'000;

  // Variable payload length (8..44 bytes) so records wrap at unaligned offsets,
  // exercising the split-memcpy path in ring_write / ring_read. The first 8
  // bytes carry the sequence number; the rest is a deterministic filler derived
  // from the sequence number so the consumer can validate the content too.
  auto make_payload = [](std::uint64_t seq) {
    const std::size_t extra = static_cast<std::size_t>(seq % 37);
    std::vector<std::byte> p(sizeof(std::uint64_t) + extra);
    std::memcpy(p.data(), &seq, sizeof(seq));
    for (std::size_t i = 0; i < extra; ++i) {
      p[sizeof(seq) + i] = static_cast<std::byte>((seq + i) & 0xFF);
    }
    return p;
  };

  // Pre-build every payload once, outside the timed region, so neither the
  // allocation nor the filler generation is charged to try_write - we want to
  // measure the queue's try_write/try_read cost only.
  std::vector<std::vector<std::byte>> payloads;
  payloads.reserve(N);
  for (std::uint64_t seq = 0; seq < N; ++seq) {
    payloads.push_back(make_payload(seq));
  }

  std::uint64_t last_fulls = 0;

  for (auto _ : state) {
    // Fresh queue per iteration so every iteration pumps exactly N messages.
    fast_queue fq;
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
        // park at the gate
      }
      std::uint64_t fulls = 0;
      for (std::uint64_t seq = 0; seq < N; ++seq) {
        std::span<const std::byte> span{payloads[seq]};
        // Back-pressure: spin until there is room. This is what drives the queue
        // to full without ever dropping a message.
        while (!prod.try_write(fq, span)) {
          ++fulls;
          std::this_thread::yield();
        }
      }
      full_events.store(fulls, std::memory_order_relaxed);
    });

    std::thread consumer_thread([&] {
      std::array<std::byte, QUEUE_SIZE> out{};
      std::uint64_t expected = 0;
      while (!go.load(std::memory_order_acquire)) {
        // park at the gate
      }
      while (expected < N) {
        auto n = cons.try_read(fq, out);
        if (!n) {
          std::this_thread::yield();
          continue;
        }
        std::uint64_t seq{};
        assert(*n >= sizeof(seq));
        std::memcpy(&seq, out.data(), sizeof(seq));
        assert(seq == expected && "out of order or lost message");

        const std::size_t extra = *n - sizeof(seq);
        for (std::size_t i = 0; i < extra; ++i) {
          assert(out[sizeof(seq) + i] == static_cast<std::byte>((seq + i) & 0xFF) &&
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
  std::println("producer hit a full queue {} times on the last iteration "
               "(back-pressure exercised)",
               last_fulls);
  std::println("test_full_ring PASSED");
}

inline void test() {
  test_basic();
  test_limits();
  BENCHMARK(test_full_ring)->UseManualTime()->Iterations(10);

  int argc{0};
  char **argv{nullptr};
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
}
} // namespace fast_queue
