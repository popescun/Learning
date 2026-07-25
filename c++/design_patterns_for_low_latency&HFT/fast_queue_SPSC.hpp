//
// Created by Nicolae Popescu on 28/10/2025.
//

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <version>

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

// A large ring (1 MiB) for the "optimized" benchmark: big enough that producer
// and consumer decouple and almost never hit full/empty, so back-pressure
// stalls vanish and throughput reflects the raw data-movement cost.
constexpr std::size_t LARGE_QUEUE_SIZE = std::size_t{1} << 20;

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
template <std::size_t Size> struct fast_queue_t {
  static_assert((Size & (Size - 1)) == 0, "queue size must be a power of two");
  static constexpr std::size_t SIZE = Size;
  static constexpr std::uint64_t MASK = Size - 1;

  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> read_counter{0};
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  alignas(CACHE_LINE_SIZE) std::array<std::byte, Size> buffer{};
};

// The default small ring used by the demos and the back-pressure benchmark.
using fast_queue = fast_queue_t<QUEUE_SIZE>;

// Spin-loop hint for busy-waiting. Tells the core we are in a spin-wait so it
// can save power and, on SMT cores, cede pipeline resources to the sibling.
// Far cheaper than std::this_thread::yield(), which traps into the kernel
// scheduler and typically deschedules the thread.
inline void spin_pause() noexcept {
#if defined(__x86_64__) || defined(__i386__)
  __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
  __asm__ __volatile__("yield" ::: "memory");
#endif
}

// Copy n bytes into the ring starting at logical(absolute) counter `counter`, wrapping around the
// physical end of the buffer when necessary (circular write).
template <class Q>
inline void ring_write(Q &fq, std::uint64_t counter, const std::byte *src, std::size_t n) {
  const auto index = static_cast<std::size_t>(counter & Q::MASK);
  const std::size_t first = std::min(n, Q::SIZE - index);
  std::memcpy(fq.buffer.data() + index, src, first);
  if (n > first) { // the record straddles the end -> continue at the start
    std::memcpy(fq.buffer.data(), src + first, n - first);
  }
}

// Copy n bytes out of the ring starting at logical(absolute) counter `counter`, wrapping around the
// physical end of the buffer when necessary (circular read).
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
   * Try to write one message. Returns false (nothing written) when the queue
   * does not have room for the whole record - this is the limit check that
   * gives us back-pressure and guarantees the consumer never loses data.
   */
  template <class Q> bool try_write(Q &fq, std::span<const std::byte> payload) {
    const auto payload_size = static_cast<header_t>(payload.size());
    const std::size_t record_size = sizeof(header_t) + payload_size;
    assert(record_size <= Q::SIZE && "message larger than the whole queue");

    // Check the free space against the limit. First use the cached tail to
    // avoid touching the consumer's cache line on every call; only refresh from
    // the shared counter if that suggests we might be full.
    std::uint64_t bytes_available_to_read = write_counter - read_counter;
    if (bytes_available_to_read + record_size > Q::SIZE) {
      read_counter = fq.read_counter.load(std::memory_order_acquire);
      bytes_available_to_read = write_counter - read_counter;
      if (bytes_available_to_read + record_size > Q::SIZE) {
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

/**
 * A borrowed, in-place view of one message still sitting in the ring buffer, returned by
 * the zero-copy read path (`consumer::try_read_view`). The consumer processes the payload
 * WITHOUT it being copied out.
 *
 * Because storage is circular a record can straddle the physical end of the buffer, so the
 * payload may arrive in up to two contiguous pieces: `first`, then `second` at the buffer's
 * start. For a message that does not wrap, `second` is empty. The bytes are valid only until
 * the read is committed (`consumer::commit_read`) - after that the producer may reuse the
 * space, so the consumer must finish reading first.
 */
struct read_view {
  std::span<const std::byte> first;
  std::span<const std::byte> second;

  std::size_t size() const noexcept { return first.size() + second.size(); }
  bool wrapped() const noexcept { return !second.empty(); }
};

struct consumer {
  /**
   * Try to read one message into `out`. Returns the number of payload bytes
   * read, or std::nullopt when the queue is empty.
   */
  template <class Q> std::optional<std::size_t> try_read(Q &fq, std::span<std::byte> out) {
    assert(pending_record == 0 && "an uncommitted zero-copy view is still outstanding");
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

  /**
   * Zero-copy read. Returns a borrowed `read_view` of the next message IN the ring - the
   * payload is NOT copied out - or std::nullopt when the queue is empty. This is a
   * two-phase API: after processing the view's bytes, call commit_read() to release the
   * space back to the producer. The view stays valid only until that commit, and exactly
   * one commit_read() must follow each successful try_read_view() (a consumer uses either
   * try_read or the view API, not both interleaved).
   *
   * Only the 4-byte length header is copied (into a local, so a header that itself straddles
   * the end is handled); the payload - the bulk - is exposed in place, saving the ring->out
   * copy that try_read performs.
   */
  template <class Q> std::optional<read_view> try_read_view(Q &fq) {
    assert(pending_record == 0 && "previous try_read_view was not committed");
    // Empty check, same cached-first / refresh-on-demand trick as try_read.
    if (read_counter == write_counter) {
      write_counter = fq.write_counter.load(std::memory_order_acquire);
      if (read_counter == write_counter) {
        return std::nullopt; // nothing to read
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

    // Remember the record size but DON'T advance/publish yet: the producer must not reuse
    // this space until the consumer has finished reading it in place (commit_read).
    pending_record = sizeof(header_t) + plen;
    return v;
  }

  /**
   * Release the message from the last try_read_view back to the producer: advance the tail
   * and publish. Must be called exactly once after a successful try_read_view().
   */
  template <class Q> void commit_read(Q &fq) {
    assert(pending_record != 0 && "commit_read without a matching try_read_view");
    read_counter += pending_record;
    pending_record = 0;
    // Publish: the producer may now reuse the space we just finished reading in place.
    fq.read_counter.store(read_counter, std::memory_order_release);
  }

  std::uint64_t read_counter{0};  // private copy of the tail
  std::uint64_t write_counter{0}; // last observed head (producer progress)
  std::size_t pending_record{0};  // size of a peeked-but-not-committed record (0 = none)
};

} // namespace fast_queue
