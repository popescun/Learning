// C++ memory model & std::atomic memory orderings — runnable examples.
//
// Header-only, following the repo convention: namespace + inline test().
// Included from main.cpp and built via the CMakeLists *.hpp glob.
//
// (compile the project with -fsanitize=thread to let TSan flag the data
//  race in example 1)

#include <atomic>
#include <cassert>
#include <print>
#include <thread>
#include <vector>

namespace memory_model_order {

// ---------------------------------------------------------------------------
// 1. Data race == undefined behavior.
//
// Two threads touch `ready`/`x` with no synchronization. The compiler and CPU
// are free to reorder the two stores in Thread A, so Thread B can observe
// ready==true while x is still 0. This is UB, not merely a "stale read".
// Shown only to be understood and avoided — do NOT write code like this.
// ---------------------------------------------------------------------------
inline void data_race_broken() {
  int x = 0;
  bool ready = false;

  std::thread producer([&] {
    x = 42;       // (1) may become visible AFTER (2)
    ready = true; // (2)
  });

  std::thread consumer([&] {
    while (!ready) { // may spin forever, or...
    }
    // std::println("[race] x = {} (no guarantee it is 42)", x);
  });

  producer.join();
  consumer.join();
}

// ---------------------------------------------------------------------------
// 2. release / acquire — the correct fix.
//
// The `release` store publishes everything the producer did before it; the
// matching `acquire` load guarantees the consumer sees those writes. This
// creates a happens-before edge: (x = 42) happens-before (read of x).
// ---------------------------------------------------------------------------
inline void release_acquire() {
  int x = 0; // plain int, protected by the atomic flag
  std::atomic<bool> ready{false};

  std::thread producer([&] {
    x = 42;                                       // non-atomic write
    ready.store(true, std::memory_order_release); // publishes x
  });

  std::thread consumer([&] {
    while (!ready.load(std::memory_order_acquire)) {
    }
    // Guaranteed: x == 42.
    // std::println("[rel/acq] x = {}", x);
    assert(x == 42);
  });

  producer.join();
  consumer.join();
}

// ---------------------------------------------------------------------------
// 3. seq_cst — the safe default.
//
// Sequentially-consistent operations behave as if all threads agree on one
// single total order of atomic operations. Easiest to reason about, and the
// default when you omit the ordering argument (ready.store(true)).
//
// Classic result: at least one of the two consumers must observe the other's
// flag, so "both saw nothing" is impossible under seq_cst.
// ---------------------------------------------------------------------------
inline void seq_cst() {
  std::atomic<bool> a{false};
  std::atomic<bool> b{false};
  int saw_a = 0, saw_b = 0;

  std::thread t1([&] {
    a.store(true);    // default == memory_order_seq_cst
    saw_b = b.load(); // did we see b?
  });
  std::thread t2([&] {
    b.store(true);
    saw_a = a.load(); // did we see a?
  });

  t1.join();
  t2.join();

  // Under seq_cst it is impossible for both loads to read false.
  // std::println("[seq_cst] saw_b={} saw_a={} (never both 0)", saw_b, saw_a);
  assert(saw_a == 1 || saw_b == 1);
}

// ---------------------------------------------------------------------------
// 4. relaxed — atomicity without ordering.
//
// A shared counter only needs each increment to be indivisible; the order in
// which increments become visible does not matter. relaxed gives atomicity
// with the least synchronization overhead. The final total is exact.
//
// Note: relaxed is fine HERE precisely because no other memory is published
// through this atomic. Do not use it to hand off data between threads.
// ---------------------------------------------------------------------------
inline void relaxed_counter() {
  std::atomic<int> counter{0};
  constexpr int kThreads = 8;
  constexpr int kPerThread = 100'000;

  std::vector<std::thread> workers;
  for (int i = 0; i < kThreads; ++i) {
    workers.emplace_back([&] {
      for (int n = 0; n < kPerThread; ++n) {
        counter.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }
  for (auto &w : workers) {
    w.join();
  }

  // std::println("[relaxed] counter = {} (expected {})", counter.load(),
               // kThreads * kPerThread);
  assert(counter.load() == kThreads * kPerThread);
}

inline void test() {
  data_race_broken(); // may print garbage / hang on weak hardware — that's UB
  release_acquire();
  seq_cst();
  relaxed_counter();
}

} // namespace memory_model_order