//
// Created by Nicolae Popescu on 28/10/2025.
//

#pragma once

#include <array>
#include <print>
#include <random>
#include <span>
#include <thread>

#ifdef __cpp_lib_hardware_interference_size
#include <new>
#define CACHE_LINE_SIZE std::hardware_destructive_interference_size
#elifdef __aarch64__ &&__APPLE__
#define CACHE_LINE_SIZE 128 // Apple Silicon
#elifdef __x86_64__ || __i386__ || __aarch64__
#define CACHE_LINE_SIZE 64
#else
#define CACHE_LINE_SIZE 64 // safe default
#endif

namespace fast_queue {

constexpr auto QUEUE_SIZE = 10 * CACHE_LINE_SIZE;
/**
* Both counters are written by the producer, read by the consumers(s).
  Before/after write operation, both counters contain the same value.

 Intervals:
  - [write_counter, read_counter] defines the interval where data can be read
out.
  - [read_counter, write_counter] defines the interval where data is being
written to.
 */
struct fast_queue {
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> read_counter{0};
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  alignas(CACHE_LINE_SIZE) std::array<std::byte, QUEUE_SIZE> buffer;
};

struct producer {

  void write(fast_queue &fq, const std::span<std::byte> &buffer) {
    std::println("producer::write...");
    // we write into the queue: buffer.size + buffer.data
    const std::uint64_t payload_size = sizeof(std::int32_t) + buffer.size();
    write_counter += payload_size;

    // todo: we must have checker for over-flowing of the buffer and resetting
    // the counters

    fq.write_counter.store(write_counter, std::memory_order_release);
    // now the interval [read_counter, write_counter] is reserved
    // consumer still can read the interval [write_counter, read_counter]

    const auto data_size = static_cast<std::int32_t>(buffer.size());
    // memcpy is safe by itself as it is only one producer writing at non-shared
    // memory at the writing time it's also safe and lock-free encode the new
    // data size
    std::memcpy(next_element, &data_size, sizeof(data_size));
    // write new data
    std::memcpy(next_element + sizeof(data_size), buffer.data(), data_size);

    fq.read_counter.store(write_counter, std::memory_order_release);
    // now the whole buffer is available for the consumer as read_counter ==
    // write_counter

    next_element += payload_size;
  }

  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  std::byte *next_element = nullptr;
};

struct consumer {
  /**
   * Gets #bytes read, 0 if nothing to read.
   */
  bool try_read(const fast_queue &fq, const std::span<std::byte> &buffer) {
    std::println("consumer::try_read...");
    if (read_counter == fq.read_counter.load(std::memory_order_acquire)) {
      return false;
    }

    std::int32_t data_size;
    std::memcpy(&data_size, next_element, sizeof(data_size));

    auto write_counter = fq.write_counter.load(std::memory_order_acquire);
    assert(write_counter - read_counter <= fq.buffer.size() &&
           "queue overflow");
    assert(data_size <= buffer.size() && "buffer space isn’t large enough");

    std::memcpy(buffer.data(), next_element + sizeof(data_size), data_size);

    const std::uint64_t payload_size = sizeof(data_size) + buffer.size();
    read_counter += payload_size;
    next_element += payload_size;

    write_counter = fq.write_counter.load(std::memory_order_acquire);
    assert(write_counter - read_counter <= fq.buffer.size() &&
           "queue overflow");

    return true;
  }

  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> read_counter{0};
  std::byte *next_element = nullptr;
};

template <class T> std::array<std::byte, sizeof(T)> to_bytes(const T &obj) {
  static_assert(std::is_trivially_copyable_v<T>);
  std::array<std::byte, sizeof(T)> buf{};
  std::memcpy(buf.data(), &obj, sizeof(T)); // memcpy is the well-defined way
  return buf;
}

template <class T> T from_bytes(const std::array<std::byte, sizeof(T)> &buf) {
  static_assert(std::is_trivially_copyable_v<T>);
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

void test1() {
  fast_queue fq;
  producer p;
  p.next_element = fq.buffer.data();
  consumer c;
  c.next_element = fq.buffer.data();

  Quote q{1'700'000'000ULL, 101.25, 500, "AAPL"};
  auto bytes = to_bytes(q);
  std::span<std::byte> data{bytes};
  p.write(fq, data);

  std::array<std::byte, sizeof(Quote)> buf{};
  auto ret = c.try_read(fq, buf);
  std::println("ret:{}", ret);
  Quote q1{};
  if (ret) {
    q1 = from_bytes<Quote>(buf);
  }
  std::println("q1 ts:{}, size:{}, price:{}, symbol:{}", q1.ts, q1.size,
               q1.price, q1.symbol);
}

void test2() {
  fast_queue fq;
  producer p;
  p.next_element = fq.buffer.data();
  consumer c;
  c.next_element = fq.buffer.data();

  std::thread t2([&fq, &c]() {
    std::println("t2 starting...");
    std::array<std::byte, sizeof(Quote)> buf{};
    while (true) {
      while (!c.try_read(fq, buf)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      auto q = from_bytes<Quote>(buf);
      std::println("q ts:{}, size:{}, price:{}, symbol:{}", q.ts, q.size,
                   q.price, q.symbol);
    }
    std::println("t2 terminating...");
  });

  std::thread t1([&fq, &p]() {
    std::println("t1 starting...");
    Quote q{1'700'000'000ULL, 101.25, 500, "AAPL"};
    auto bytes = to_bytes(q);
    std::span<std::byte> data{bytes};
    p.write(fq, data);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    q.price = 111.25;
    bytes = to_bytes(q);
    std::span<std::byte> data1{bytes};
    p.write(fq, data1);
    std::println("t1 terminating...");
  });

  t1.join();
  t2.join();
}

inline void test() {
  // test1();
  test2();
}
} // namespace fast_queue
