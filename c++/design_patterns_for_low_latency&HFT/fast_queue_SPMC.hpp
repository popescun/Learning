//
// Created by Nicolae Popescu on 28/10/2025.
//
// =====================================================================================
//  fast_queue_SPMC.hpp — single-producer / multi-consumer BROADCAST ring buffer
//                        (Disruptor-style multicast, "Option B")
// =====================================================================================
//
// This is the multi-consumer sibling of the SPSC ring in fast_queue_SPSC.hpp. It keeps
// the same core idea — absolute, never-wrapped byte counters, a power-of-two buffer
// addressed by a bit-mask, acquire/release publishing, no locks and no CAS — and
// generalises it to N consumers.
//
// -------------------------------------------------------------------------------------
//  Semantics: BROADCAST (fan-out), not work-sharing
// -------------------------------------------------------------------------------------
//  Every consumer receives EVERY message. Use this when the N consumers are DIFFERENT
//  (a logger, a risk check, a strategy) and each needs the full stream. It is NOT the
//  "competing consumers / load-balancing" pattern where each message is handled by
//  exactly one worker — that is a separate design (partition into per-worker SPSC lanes,
//  or CAS on a shared tail, which reintroduces cross-core contention and is avoided here).
//
// -------------------------------------------------------------------------------------
//  Why "Option B" (single shared buffer) instead of N independent SPSC queues
// -------------------------------------------------------------------------------------
//  Option A would fan the message into N separate SPSC queues: simple (reuses the SPSC
//  code untouched) but the producer memcpys the payload N times and its hot write-set
//  grows with N. Option B keeps ONE buffer: the producer writes each message ONCE, and
//  the buffer lines are then read-SHARED across the consumer cores. Multiple cores
//  holding the same line in the "Shared" coherency state cost nothing to each other (no
//  invalidation traffic), so fan-out is cheap — this is the cache-friendly direction and
//  scales better as N grows.
//
// -------------------------------------------------------------------------------------
//  The one structural change vs. SPSC: N read counters + a min() reuse gate
// -------------------------------------------------------------------------------------
//  SPSC has one read_counter (the single consumer's tail). Here each consumer owns its
//  OWN read_counter, each on its own cache line (no false sharing between consumers, and
//  the producer scanning all N never ping-pongs a single line). The producer may reuse
//  (overwrite) buffer space only up to
//
//        min(read_counter[0..N-1])          // the SLOWEST consumer gates reuse
//
//  because a byte is only safe to overwrite once EVERY consumer has read past it. That
//  single min() is the whole algorithmic difference from SPSC.
//
// -------------------------------------------------------------------------------------
//  Slow-consumer policy — this sketch is LOSSLESS (back-pressure)
// -------------------------------------------------------------------------------------
//  With the min() gate, a slow consumer back-pressures the PRODUCER: try_write returns
//  false (full) until the laggard catches up. No message is ever lost — but one slow
//  consumer stalls everyone (head-of-line blocking). This is the simplest correct first
//  cut and mirrors the SPSC guarantee.
//
//  The realistic market-data alternative is OVERWRITE / DROP: the producer never waits,
//  and a consumer that gets "lapped" (write_counter - my_read > SIZE) detects the gap,
//  skips ahead, and counts the loss. That trades the head-of-line stall for lossy
//  delivery on the slow lane. It is sketched — but not implemented — in the notes on
//  try_read below, because it changes the correctness contract (torn reads become
//  possible and must be detected), not just a policy knob.
//
// -------------------------------------------------------------------------------------
//  N is a COMPILE-TIME template parameter
// -------------------------------------------------------------------------------------
//  NConsumers is fixed at compile time (like Size), so the per-consumer counter array is
//  a std::array with a static, cache-stable layout and zero indirection — matching the
//  fast_queue_t<Size> style. A runtime-N variant would heap-allocate the counter array
//  and add an indirection; not needed for the fixed consumer sets typical in HFT.
//
// -------------------------------------------------------------------------------------
//  Memory ordering — identical shape to SPSC, just N of the second pair
// -------------------------------------------------------------------------------------
//    producer: write bytes ; store write_counter (release)
//    consumer: load write_counter (acquire) ; read bytes            // data visibility
//
//    consumer: read bytes ; store read_counter[id] (release)
//    producer: load read_counter[0..N] (acquire) ; overwrite bytes  // reuse safety
//
//  The producer can only reach a slot to overwrite once the min() gate says all N
//  consumers passed it, so no consumer is mid-reading a slot the producer touches — no
//  torn reads, same guarantee as SPSC.
//

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new> // std::hardware_destructive_interference_size
#include <optional>
#include <span>

// Kept inside the namespace (below) so this header can coexist in one TU with
// fast_queue_SPSC.hpp, which defines a CACHE_LINE_SIZE of its own at global scope.
namespace fast_queue_spmc {

#if defined(__cpp_lib_hardware_interference_size)
inline constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#elif defined(__aarch64__) && defined(__APPLE__)
inline constexpr std::size_t CACHE_LINE_SIZE = 128; // Apple Silicon
#else
inline constexpr std::size_t CACHE_LINE_SIZE = 64; // safe default
#endif

// Small by default so tests wrap the ring and exercise the reuse gate quickly.
constexpr std::size_t QUEUE_SIZE = 1024;

// [int32 length][payload bytes], exactly as in the SPSC design.
using header_t = std::int32_t;

// Spin-loop hint for busy-waiting (see fast_queue_SPSC.hpp for the rationale).
inline void spin_pause() noexcept {
#if defined(__x86_64__) || defined(__i386__)
  __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
  __asm__ __volatile__("yield" ::: "memory");
#endif
}

/**
 * Single-producer / N-consumer broadcast ring.
 *
 *  - write_counter        : total bytes committed by the producer (head). Written by the
 *                           producer, read by every consumer.
 *  - read_counter[i]      : total bytes consumed by consumer i (its private tail).
 *                           Written by consumer i, read by the producer for the min() gate.
 *  - buffer               : the shared circular storage, read by all consumers.
 *
 * Every counter sits on its own cache line. The producer overwrites only up to
 * min(read_counter[*]); until then a slot is still owned by at least one consumer.
 */
template <std::size_t Size, std::size_t NConsumers> struct spmc_queue_t {
  static_assert((Size & (Size - 1)) == 0, "queue size must be a power of two");
  static_assert(NConsumers >= 1, "need at least one consumer");
  static constexpr std::size_t SIZE = Size;
  static constexpr std::uint64_t MASK = Size - 1;
  static constexpr std::size_t N = NConsumers;

  // One cache line per counter so no two writers (or the producer's N-way scan) share a
  // line. padded_counter wraps the atomic so the array elements are individually aligned.
  struct alignas(CACHE_LINE_SIZE) padded_counter {
    std::atomic<std::uint64_t> value{0};
  };

  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  std::array<padded_counter, NConsumers> read_counter{};
  alignas(CACHE_LINE_SIZE) std::array<std::byte, Size> buffer{};
};

// --- circular copy helpers (generic over the queue type, identical to SPSC) -----------
template <class Q>
inline void ring_write(Q &fq, std::uint64_t counter, const std::byte *src, std::size_t n) {
  const auto index = static_cast<std::size_t>(counter & Q::MASK);
  const std::size_t first = std::min(n, Q::SIZE - index);
  std::memcpy(fq.buffer.data() + index, src, first);
  if (n > first) {
    std::memcpy(fq.buffer.data(), src + first, n - first);
  }
}

template <class Q>
inline void ring_read(const Q &fq, std::uint64_t counter, std::byte *dst, std::size_t n) {
  const auto index = static_cast<std::size_t>(counter & Q::MASK);
  const std::size_t first = std::min(n, Q::SIZE - index);
  std::memcpy(dst, fq.buffer.data() + index, first);
  if (n > first) {
    std::memcpy(dst + first, fq.buffer.data(), n - first);
  }
}

struct producer {
  /**
   * Write one message, visible to ALL consumers. Returns false (nothing written) when the
   * ring cannot hold the whole record because the SLOWEST consumer has not freed enough
   * space yet — this is the lossless back-pressure gate.
   */
  template <class Q> bool try_write(Q &fq, std::span<const std::byte> payload) {
    const auto payload_size = static_cast<header_t>(payload.size());
    const std::size_t record_size = sizeof(header_t) + payload_size;
    assert(record_size <= Q::SIZE && "message larger than the whole queue");

    // Free space is gated by the slowest consumer. Use the cached min first (like SPSC's
    // cached tail) so the common path never touches the consumers' lines; only refresh —
    // scanning all N counters and taking the min — when the cache says we might be full.
    std::uint64_t bytes_in_flight = write_counter - cached_min_read;
    if (bytes_in_flight + record_size > Q::SIZE) {
      cached_min_read = load_min_read(fq);
      bytes_in_flight = write_counter - cached_min_read;
      if (bytes_in_flight + record_size > Q::SIZE) {
        return false; // slowest consumer still behind -> genuinely full
      }
    }

    // Write once; every consumer will read these same bytes (read-shared, cheap fan-out).
    ring_write(fq, write_counter, reinterpret_cast<const std::byte *>(&payload_size),
               sizeof(payload_size));
    ring_write(fq, write_counter + sizeof(payload_size), payload.data(), payload.size());

    write_counter += record_size;
    // Publish once: release pairs with each consumer's acquire load of write_counter.
    fq.write_counter.store(write_counter, std::memory_order_release);
    return true;
  }

  // Scan all N consumer tails and return the minimum (the reuse gate). Each load is
  // acquire so that, before we overwrite a slot, we have observed the slowest consumer
  // actually finishing its read of it.
  template <class Q> static std::uint64_t load_min_read(const Q &fq) {
    std::uint64_t m = std::numeric_limits<std::uint64_t>::max();
    for (const auto &rc : fq.read_counter) {
      m = std::min(m, rc.value.load(std::memory_order_acquire));
    }
    return m;
  }

  std::uint64_t write_counter{0};   // private copy of the head (producer is sole writer)
  std::uint64_t cached_min_read{0}; // last observed min() of the consumer tails
};

/**
 * A borrowed, in-place view of one message still in the ring buffer, returned by the
 * zero-copy read path (`consumer::try_read_view`). Same shape as the SPSC read_view: because
 * storage is circular the payload may arrive in up to two contiguous pieces - `first`, then
 * `second` at the buffer start; `second` is empty when the record does not wrap.
 *
 * The bytes are valid only until THIS consumer commits (`commit_read`). The producer's min()
 * reuse gate includes this consumer's (not-yet-advanced) tail, so it cannot overwrite the
 * peeked message while the view is outstanding - the same guarantee the SPSC gate gives.
 */
struct read_view {
  std::span<const std::byte> first;
  std::span<const std::byte> second;

  std::size_t size() const noexcept { return first.size() + second.size(); }
  bool wrapped() const noexcept { return !second.empty(); }
};

struct consumer {
  // Each consumer is bound to its own slot in the read_counter array. Ids must be the
  // distinct values 0..N-1 across the consumer set.
  explicit consumer(std::size_t id) : id{id} {}

  /**
   * Read the next message for THIS consumer into `out`, or std::nullopt if this consumer
   * has already caught up to the producer. Each consumer advances independently through
   * the same buffer, so slow and fast consumers see the full stream at their own pace.
   *
   * The read path is essentially the SPSC try_read — the elegance of Option B is that
   * only publishing changes (to this consumer's own counter slot).
   */
  template <class Q> std::optional<std::size_t> try_read(Q &fq, std::span<std::byte> out) {
    assert(pending_record == 0 && "an uncommitted zero-copy view is still outstanding");
    // Empty check for this consumer: cached head first, refresh only when it looks empty.
    if (read_counter == cached_write) {
      cached_write = fq.write_counter.load(std::memory_order_acquire);
      if (read_counter == cached_write) {
        return std::nullopt; // this consumer has read everything published so far
      }
    }

    // NOTE (overwrite/drop variant, NOT active in this lossless sketch): a lossy design
    // would here detect being lapped — (cached_write - read_counter > Q::SIZE) — resync
    // read_counter to the oldest still-live position, count the dropped bytes, and only
    // then read. It would also need a re-validation of write_counter AFTER copying (a
    // seqlock-style check) because without the min() gate the producer could overwrite a
    // slot mid-read. The lossless gate below makes all of that unnecessary.

    header_t payload_size{};
    ring_read(fq, read_counter, reinterpret_cast<std::byte *>(&payload_size), sizeof(payload_size));
    assert(payload_size >= 0 && static_cast<std::size_t>(payload_size) <= out.size() &&
           "output buffer isn't large enough for the message");

    ring_read(fq, read_counter + sizeof(payload_size), out.data(),
              static_cast<std::size_t>(payload_size));

    const std::size_t record_size = sizeof(payload_size) + static_cast<std::size_t>(payload_size);
    read_counter += record_size;
    // Publish this consumer's progress. Once ALL consumers pass a byte, the producer's
    // min() gate lets it reuse that space.
    fq.read_counter[id].value.store(read_counter, std::memory_order_release);
    return static_cast<std::size_t>(payload_size);
  }

  /**
   * Zero-copy read for THIS consumer: returns a borrowed read_view of the next message IN the
   * ring (no copy of the payload), or std::nullopt when this consumer has caught up to the
   * producer. Two-phase like the SPSC version: process the view's bytes, then commit_read() to
   * advance this consumer's tail and let the producer's min() gate eventually reuse the space.
   * Exactly one commit_read() must follow each successful try_read_view() (a consumer uses
   * either try_read or the view API, not both interleaved).
   *
   * Only the 4-byte length header is copied (into a local, so a wrapped header is handled); the
   * payload - the bulk - is exposed in place, and because the buffer lines are read-SHARED
   * across consumers this stays a cheap fan-out (no per-consumer copy).
   */
  template <class Q> std::optional<read_view> try_read_view(Q &fq) {
    assert(pending_record == 0 && "previous try_read_view was not committed");
    if (read_counter == cached_write) {
      cached_write = fq.write_counter.load(std::memory_order_acquire);
      if (read_counter == cached_write) {
        return std::nullopt; // this consumer has read everything published so far
      }
    }

    header_t payload_size{};
    ring_read(fq, read_counter, reinterpret_cast<std::byte *>(&payload_size), sizeof(payload_size));
    assert(payload_size >= 0);

    // Expose the payload in place, splitting into (at most) two pieces if it wraps the end.
    const std::uint64_t payload_start = read_counter + sizeof(header_t);
    const auto index = static_cast<std::size_t>(payload_start & Q::MASK);
    const auto plen = static_cast<std::size_t>(payload_size);
    const std::size_t first_len = std::min(plen, Q::SIZE - index);

    read_view v{};
    v.first = std::span<const std::byte>{fq.buffer.data() + index, first_len};
    if (plen > first_len) { // straddles the end -> second piece at the buffer start
      v.second = std::span<const std::byte>{fq.buffer.data(), plen - first_len};
    }

    // Remember the record size but DON'T advance/publish yet: the producer's min() gate keeps
    // this consumer's peeked bytes alive only while this tail has not moved past them.
    pending_record = sizeof(header_t) + plen;
    return v;
  }

  /**
   * Release the message from the last try_read_view: advance THIS consumer's tail and publish.
   * Must be called exactly once after a successful try_read_view().
   */
  template <class Q> void commit_read(Q &fq) {
    assert(pending_record != 0 && "commit_read without a matching try_read_view");
    read_counter += pending_record;
    pending_record = 0;
    fq.read_counter[id].value.store(read_counter, std::memory_order_release);
  }

  std::size_t id;                // which read_counter slot this consumer owns (0..N-1)
  std::uint64_t read_counter{0}; // this consumer's private tail
  std::uint64_t cached_write{0}; // last observed head (producer progress)
  std::size_t pending_record{0}; // size of a peeked-but-not-committed record (0 = none)
};

} // namespace fast_queue_spmc
