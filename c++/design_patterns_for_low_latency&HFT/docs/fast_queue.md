# Fast Queue — a lock-free SPSC ring buffer

`fast_queue_SPSC.hpp` implements a **single-producer / single-consumer (SPSC)**,
lock-free, byte-oriented **ring buffer** designed for low-latency / HFT-style
message passing between two threads (e.g. a market-data feed thread and a
strategy thread).

It carries **variable-sized, length-prefixed messages**, reuses its storage
**circularly**, applies **back-pressure** so no message is ever lost, and
synchronizes the two threads with nothing more than two atomic counters and
acquire/release ordering — no mutexes, no CAS loops.

### Source layout

The implementation and its tests live in **separate headers** so the queue reads
as a clean, self-contained component:

| File | Contents |
|------|----------|
| `fast_queue_SPSC.hpp` | **Implementation only** — the ring buffer, `producer`, `consumer`, the `ring_write`/`ring_read` helpers, and `spin_pause`. |
| `fast_queue_SPSC_test.hpp` | **Tests & benchmarks** — the demos, the `to_bytes`/`from_bytes` serialization helpers, the demo POD types (`Quote`, `latency_msg`), and the `fast_queue::test()` entry point. Reopens `namespace fast_queue`. |

`main.cpp` includes `fast_queue_SPSC_test.hpp` and calls `fast_queue::test()`.

> A separate **multi-consumer (SPMC) broadcast** variant is sketched in
> `fast_queue_SPMC.hpp` — one shared buffer, per-consumer read counters, every
> consumer sees every message. See that file's header comment for the design.

The ring is **parameterized on its capacity** — `fast_queue_t<Size>` — so the same
code serves both a tiny 1 KB ring (to force wraps and back-pressure in tests) and
a 1 MB ring (to measure raw throughput). `fast_queue` is an alias for the default
small ring; see §1.

---

## 1. Data model

```cpp
template <std::size_t Size>
struct fast_queue_t {
  static_assert((Size & (Size - 1)) == 0, "queue size must be a power of two");
  static constexpr std::size_t SIZE = Size;
  static constexpr std::uint64_t MASK = Size - 1;

  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> read_counter{0};
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  alignas(CACHE_LINE_SIZE) std::array<std::byte, Size> buffer{};
};

using fast_queue = fast_queue_t<QUEUE_SIZE>;   // the default small ring
```

The ring is a **template on its byte capacity `Size`**, which must be a power of
two (static-asserted). It publishes the capacity and address mask as
`static constexpr SIZE` / `MASK` so the helpers, producer, and consumer can be
written once and work for any size. Two ready-made sizes are used:

| Alias / constant | Size | Used by |
|------------------|------|---------|
| `fast_queue` (= `fast_queue_t<QUEUE_SIZE>`) | `QUEUE_SIZE` = **1 KB** | demos + back-pressure benchmark (small ring forces wraps & full/empty collisions) |
| `fast_queue_t<LARGE_QUEUE_SIZE>` | `LARGE_QUEUE_SIZE` = **1 MB** | "optimized" throughput benchmark (producer/consumer decouple, back-pressure vanishes) |

The whole state is three fields:

| Field           | Owner (writer) | Reader   | Meaning                                   |
|-----------------|----------------|----------|-------------------------------------------|
| `write_counter` | producer       | consumer | total bytes ever **committed** (the head) |
| `read_counter`  | consumer       | producer | total bytes ever **consumed** (the tail)  |
| `buffer`        | producer       | consumer | the raw circular storage                  |

### Monotonic counters

Both counters are **absolute, monotonically increasing** 64-bit byte offsets —
they only ever go up, they are *never* wrapped or reset. A 64-bit counter of
bytes will not overflow in any realistic runtime (at 100 GB/s it would take
~58 years), so we never have to worry about counter wrap.

The number of bytes currently *available to read* (written but not yet read) is
simply:

```
bytes_available_to_read = write_counter - read_counter   // always in [0, QUEUE_SIZE]
```

From that single quantity we derive the two boundary states:

```
empty  ⇔  write_counter == read_counter          (bytes_available_to_read == 0)
full   ⇔  write_counter - read_counter == QUEUE_SIZE
```

Because empty vs. full is decided by the **difference** of two ever-growing
counters — not by comparing wrapped read/write *positions* — there is no
ambiguity between "empty" and "full" and therefore **no wasted slot**. The
entire `QUEUE_SIZE` bytes are usable. (Classic pointer-based ring buffers store
two *wrapped* indices, which can't tell empty from full on their own; they need
one extra bit of state — commonly by leaving a slot empty. This design gets that
bit for free from the counter difference.)

#### Convention

Throughout, `head` and `tail` are **byte counters**. A write or read transfers a
whole message at once, so `head` and `tail` don't step one byte at a time —
each advances by the **size of the message in bytes**: `head` by the bytes
written on every write, `tail` by the bytes read on every read. The per-byte
framing used in some of the examples below is just an illustrative special case
(a one-byte message); the counters always move in message-sized steps. Only the
producer moves `head`; only the consumer moves `tail`. This design uses
**advance-on-write**: the message is written starting at `head`'s slot and
*then* `head` is advanced past it (never the reverse). Emptiness and fullness are
questions about the *gap* between the two counters.

#### Why two wrapped indices can't tell empty from full

Suppose you store `head` and `tail` as **wrapped** indices in `[0, N)`, the
textbook ring buffer. The pair then reveals only their difference
`d = (head - tail) mod N` — `N` possible values — while the fill level has
`N + 1` (`0, 1, … N` bytes). `N` labels cannot cover `N + 1` states, so exactly
one pair of fill levels collides onto the same `head == tail`.

*Which* pair collides is decided purely by the fencepost, and you can slide it but
never remove it:

- **advance-on-write** (write at `head`, then `head += 1`) — the collision is at
  the **top**. Writing `N` bytes wraps `head` all the way back onto `tail`:

  ```
  N = 8, start empty:  head = tail = 3
  write 8 bytes  →  head = (3 + 8) mod 8 = 3 == tail    // FULL looks empty
  ```

- **write-in-place** (write at `head`, `head += 1` only *between* bytes) — the
  collision moves to the **bottom**. The very first byte leaves `head` put:

  ```
  N = 8, start empty:  head = tail = 3
  write 1 byte   →  head = (3 + 0) mod 8 = 3 == tail    // 1 byte looks empty
  ```

Either fencepost leaves one fill level unrepresentable — the "wasted slot,"
whichever end it lands on. No test on two wrapped indices can recover it; the
distinguishing bit was never stored.

To break the tie you need **one extra bit of state** beyond two wrapped indices.
Common options: waste a slot (declare full at `(head + 1) % N == tail`, capping
usable bytes at `N − 1`); keep a separate byte count `0 … N`; or **don't wrap the
counters at all** — which is what this design does.

#### Why this design uses the whole buffer: absolute counters, mask only for addressing

The key move is to **separate the two jobs a ring-buffer index performs**:

1. **Decide empty / full** — from the *absolute, never-wrapped* counters.
2. **Find the physical slot** — with `& MASK`, and *only* here.

`write_counter` and `read_counter` are monotonically increasing byte totals that
are **never reduced mod N**. Their true difference therefore lands in `[0, N]` —
all `N + 1` fill levels, with no fold:

```
bytes_available_to_read = write_counter - read_counter   // in [0, QUEUE_SIZE]
empty ⇔ difference == 0
full  ⇔ difference == QUEUE_SIZE
```

With advance-on-write and un-wrapped counters, a full queue is e.g. `head = 11`,
`tail = 3` → difference `8`, plainly distinct from empty's difference `0` — even
though both map to the *same* slot (`11 & MASK == 3 & MASK == 3`). The wrap that
sank the classic scheme is invisible to the empty/full test, because that test
never looks at a wrapped value.

Modulo enters exactly once, and only to answer "*where* in the buffer," never
"*how full*":

```
offset_in_buffer = counter & MASK     // == counter % QUEUE_SIZE
```

(see *Mapping a counter to a buffer position — a bit-mask, not a modulo* below).
Keep the counters absolute for the state decision, apply the mask solely for
addressing, and all `QUEUE_SIZE` bytes are usable — no sacrificed slot.

The trade this makes is relying on the counters not overflowing. With
`uint64_t` byte counters, even at 10 GB/s that takes ~58 years, which is why we
never worry about counter wrap. Classic 32-bit position indices dodge overflow
but pay the empty-slot tax instead; for a 64-bit queue the difference-of-counters
trade is clearly the better one.

### Mapping a counter to a buffer position — a bit-mask, not a modulo

**The offset is `counter % QUEUE_SIZE`, but we compute it as `counter &
MASK` — a single-instruction bit-mask instead of an expensive `%`
divide.** The two are identical because `QUEUE_SIZE` is a power of two.

```
offset_in_buffer = counter & MASK                 // == counter % QUEUE_SIZE
                                                  // MASK = Size - 1
```

Why they're equal: dividing by `2^k` is a shift right by `k`, and the remainder
is exactly the `k` low bits that get shifted off. `& MASK` keeps precisely
those low bits, so it *is* the modulo — for a power-of-two divisor only.

This is why the equivalence holds only when the size is a **power of two**,
which is enforced at compile time (the assert uses the same trick: a power of two
has one bit set, so `N & (N - 1) == 0`). The addressing mask lives on the queue
template as `MASK = Size - 1`:

```
constexpr std::size_t QUEUE_SIZE = 1024;
static_assert((QUEUE_SIZE & (QUEUE_SIZE - 1)) == 0, "QUEUE_SIZE must be a power of two");

template <std::size_t Size> struct fast_queue_t {
  static_assert((Size & (Size - 1)) == 0, "queue size must be a power of two");
  static constexpr std::uint64_t MASK = Size - 1;   // applied only for addressing
  // ...
};
```

### Cache-line alignment (false-sharing avoidance)

Each of the three fields is `alignas(CACHE_LINE_SIZE)`. `read_counter` and
`write_counter` are written by *different* threads; if they shared a cache line,
every producer write would invalidate the consumer's copy of the line it reads
(and vice-versa) — "false sharing" — costing a coherency round-trip on the hot
path. Putting each on its own cache line eliminates that. `CACHE_LINE_SIZE` is
resolved at compile time (128 bytes on Apple Silicon, 64 on x86, or
`std::hardware_destructive_interference_size` when the library provides it).

> **⚠️ Cache line ≠ L1 cache size.** `CACHE_LINE_SIZE` is the width of a single
> **cache line** — the fixed-size block the whole hierarchy (L1/L2/L3) moves and
> tracks coherency on — **not** the total capacity of the L1 cache. They differ
> by orders of magnitude:
>
> | Term | Typical value | What it is |
> |------|--------------|------------|
> | Cache **line** size (`CACHE_LINE_SIZE`) | 64 B (x86), 128 B (Apple Silicon) | granularity of one cache block / coherency unit |
> | L1 data cache **size** | 32–192 KB | total L1 capacity (hundreds of lines) |
>
> So `alignas(CACHE_LINE_SIZE)` means "start this object on a fresh cache-line
> boundary" (padding the rest of the line), **not** "reserve an L1's worth of
> space." The whole point is coherency granularity: because a core invalidates
> an *entire line* when it writes any byte in it, keeping `read_counter` and
> `write_counter` on separate lines stops the producer's and consumer's writes
> from ping-ponging one shared line between cores (false sharing).
>
> The Apple Silicon value is **128 B** even though its L1 line is 64 B: the
> value is deliberately conservative because the M-series prefetcher/coherency
> logic effectively operates on 128 B line pairs, so aligning to 128 also avoids
> a neighboring-line form of false sharing. This is exactly why the standard
> constant is named `hardware_destructive_interference_size` (the minimum
> spacing to avoid interference) rather than "cache line size."

---

## 2. Message framing

The buffer holds a stream of **length-prefixed records**. Each record is:

```
+-------------------+--------------------------------+
| header_t length   |   payload bytes (length bytes) |
| (int32, 4 bytes)  |                                |
+-------------------+--------------------------------+
```

`using header_t = std::int32_t;` — a 4-byte length prefix precedes every
payload. So a payload of `P` bytes occupies `record_size = sizeof(header_t) + P`
bytes in the ring. This self-describing framing is what lets the consumer read
back **variable-sized** messages and know exactly where each one ends.

---

## 3. Circular read/write helpers

Because storage is circular, a record that starts near the end of the buffer can
**straddle** the physical end and continue at the beginning. Two helpers hide
that by splitting the copy in (at most) two `memcpy`s:

```cpp
template <class Q>
inline void ring_write(Q &fq, std::uint64_t counter,
                       const std::byte *src, std::size_t n) {
  const auto index = static_cast<std::size_t>(counter & Q::MASK);
  const std::size_t first = std::min(n, Q::SIZE - index);     // bytes until end
  std::memcpy(fq.buffer.data() + index, src, first);          // part 1
  if (n > first)                                               // wrapped?
    std::memcpy(fq.buffer.data(), src + first, n - first);    // part 2 @ start
}
```

The helpers are **generic over the queue type `Q`**, taking the capacity and mask
from `Q::SIZE` / `Q::MASK` so the same code serves the 1 KB and 1 MB rings. They
take the **absolute counter** (`write_counter` / `read_counter`) and mask it down
to the physical buffer `index` themselves — the caller passes the logical
counter, not a pre-wrapped position. `ring_read` is the mirror image
(buffer → destination). `first` is how many bytes fit before the physical end; if
`n > first` the remainder wraps to index `0`. When a record fits without
wrapping, the second `memcpy` is skipped.

This split logic applies to **both** the 4-byte header and the payload, so even
the length prefix itself may straddle the boundary and is handled correctly.

---

## 4. The producer — `try_write`

```cpp
template <class Q>
bool try_write(Q &fq, std::span<const std::byte> payload);
```

Returns `true` if the message was written, `false` if the queue was too full to
hold the whole record (nothing is written in that case — messages are never
partially enqueued). The method is templated on the queue type `Q`, and the
free-space check below uses `Q::SIZE` in place of the literal `QUEUE_SIZE`.

The producer keeps two **private** (non-atomic) variables:

```cpp
std::uint64_t write_counter{0}; // its own copy of the head
std::uint64_t read_counter{0};  // last value it observed of the tail
```

### Step 1 — the limit check (back-pressure)

```cpp
const auto payload_size = static_cast<header_t>(payload.size());
const std::size_t record_size = sizeof(header_t) + payload_size;  // header + payload

std::uint64_t bytes_available_to_read = write_counter - read_counter;
if (bytes_available_to_read + record_size > QUEUE_SIZE) {         // maybe full?
  read_counter = fq.read_counter.load(std::memory_order_acquire); // refresh
  bytes_available_to_read = write_counter - read_counter;
  if (bytes_available_to_read + record_size > QUEUE_SIZE)
    return false;                                                 // truly full
}
```

`payload_size` (the 4-byte length prefix) is computed once up front and reused
both for the free-space check here and for framing in Step 2.

This is the core of *"counters checked against the limits."* We only accept the
write if `bytes_available_to_read + record_size <= QUEUE_SIZE`, i.e. the new record fits
in the free space.

Two subtleties make this both **fast** and **correct**:

- **Cached tail first.** Reading the consumer's shared `read_counter` atomic
  touches a cache line the consumer keeps writing, which is expensive. So we
  first test against the producer's private `read_counter` (a stale snapshot of
  the tail). Only if *that* says we might be full do we pay for the real atomic
  load and re-test. On the common path (queue not full) the atomic is never
  touched.

- **Addition, not subtraction, avoids unsigned underflow.** A stale private
  `read_counter` is always ≤ the real tail, so `write_counter - read_counter`
  *over*-estimates the bytes available — it can even exceed `QUEUE_SIZE`. Writing
  the test as `bytes_available_to_read + record_size > QUEUE_SIZE` keeps both sides as
  well-defined additions. The naive form `QUEUE_SIZE - bytes_available_to_read <
  record_size` would
  underflow the unsigned subtraction and wrongly report free space. The
  over-estimate only ever triggers a *refresh*, never a false accept — so it is
  conservative and safe.

### Step 2 — write the framed record

```cpp
ring_write(fq, write_counter, &payload_size, sizeof(payload_size));        // header
ring_write(fq, write_counter + sizeof(payload_size), payload.data(), payload.size());
```

At this point the bytes are in the buffer but **not yet published** — the
consumer must not see them.

### Step 3 — publish

```cpp
write_counter += record_size;                                  // advance head
fq.write_counter.store(write_counter, std::memory_order_release);
```

The single `release` store is what makes the record visible. See §6 for why the
ordering matters. Advancing the private `write_counter` before the store keeps
the producer's own bookkeeping and the published value identical (the producer is
the *only* writer of `write_counter`, so its private copy is authoritative).

---

## 5. The consumer — `try_read`

```cpp
template <class Q>
std::optional<std::size_t> try_read(Q &fq, std::span<std::byte> out);
```

Returns the payload length that was copied into `out`, or `std::nullopt` if the
queue was empty. Like `try_write`, it is templated on the queue type `Q`. The consumer keeps its own private mirror:

```cpp
std::uint64_t read_counter{0};  // its own copy of the tail
std::uint64_t write_counter{0}; // last value it observed of the head
```

### Step 1 — the empty check

```cpp
if (read_counter == write_counter) {
  write_counter = fq.write_counter.load(std::memory_order_acquire);  // refresh
  if (read_counter == write_counter)
    return std::nullopt;                                             // empty
}
```

Same cached-first / refresh-on-demand trick as the producer, in reverse: only
when the cached head says "nothing new" do we pay for the real atomic load.

The invariant `read_counter <= write_counter` always holds (the consumer only ever
advances `read_counter` up to a head value it has actually observed, and heads are
published on whole-record boundaries), so a simple `==` test correctly detects
emptiness — it can never step past the producer into unwritten bytes.

### Step 2 — read the framed record

```cpp
header_t payload_size{};
ring_read(fq, read_counter, &payload_size, sizeof(payload_size));   // 1. read length
// assert payload_size fits in `out`
ring_read(fq, read_counter + sizeof(payload_size), out.data(), payload_size); // 2. payload
```

The header is read first to learn the payload length, then exactly that many
payload bytes are copied out — correctly reconstructing a variable-sized message.
Because the producer published `write_counter` only *after* fully writing the
record, and the consumer never reads beyond `write_counter`, a complete record is
always present before it is read.

### Step 3 — publish the freed space

```cpp
read_counter += sizeof(payload_size) + payload_size;          // advance tail
fq.read_counter.store(read_counter, std::memory_order_release);
```

Advancing `read_counter` tells the producer those bytes are now free to reuse —
this is the feedback loop that makes the back-pressure in §4 work.

---

## 6. Memory ordering — why it's correct

Only two synchronization points exist, and they form two acquire/release pairs:

**Producer → Consumer (data visibility).**

```
producer:  write payload bytes ; store write_counter (release)
consumer:  load write_counter (acquire) ; read payload bytes
```

The `release` store on `write_counter` guarantees that all the buffer writes that
happened *before* it are visible to any thread that performs the matching
`acquire` load and sees the new value. So once the consumer observes an advanced
head, the record's bytes are guaranteed to be there. **No torn reads.**

**Consumer → Producer (space reuse).**

```
consumer:  read payload bytes ; store read_counter (release)
producer:  load read_counter (acquire) ; overwrite those bytes
```

The `release`/`acquire` pair on `read_counter` guarantees the producer does not
reuse (overwrite) a byte until the consumer has finished reading it.

Together these two pairs are sufficient for a correct SPSC queue with no locks
and no read-modify-write atomics. This was validated by running the test suite
under **ThreadSanitizer** with zero reported data races.

### What `acquire` and `release` actually do

`release` and `acquire` are **one-way memory barriers** that only constrain
reordering; they are not locks and never block.

- **`store(v, memory_order_release)`** — a *release* store. It guarantees that
  **every memory write the thread issued before it (in program order) is
  completed and visible before this store becomes visible** to another thread.
  Nothing above it may be reordered below it. Think of it as "publish
  everything I just wrote, *then* flip this flag."

- **`load(memory_order_acquire)`** — an *acquire* load. The mirror barrier:
  **nothing after it may be reordered before it**, and if it observes a value
  written by a release store, then everything that happened before that release
  store is guaranteed visible afterwards.

The two compose into the guarantee the queue relies on:

> **If the acquire load reads the value the release store wrote, then all writes
> the producer made before the release are visible to the consumer after the
> acquire.**

Concretely on the producer side:

```cpp
ring_write(fq, write_counter, &payload_size, ...);   // (1) header
ring_write(fq, write_counter + ..., payload.data()); // (2) payload
fq.write_counter.store(write_counter, release);      // (3) publish
```

The `release` at (3) forbids (1) and (2) from being reordered after it and
forces them visible first. So when the consumer's `acquire` load sees the
advanced `write_counter`, the bytes behind it are guaranteed present — **no torn
read**. Without the barrier the CPU/compiler would be free to make (3) visible
before (1)/(2) landed, and the consumer could read uninitialized bytes.

### Why acquire/release and not `relaxed` or `seq_cst`

- **`relaxed`** gives atomicity but **no ordering** — it makes no promise about
  when the surrounding buffer writes become visible relative to the counter. The
  reordering bug above would be allowed, so it is *incorrect* for publishing
  data. (It is fine for the pure statistics counters, e.g. `full_events`, which
  no other data depends on — those use `relaxed`.)

- **`seq_cst`** (the default) is *also correct* but *stronger than needed*: it
  additionally imposes a single total order across **all** `seq_cst` operations,
  which typically compiles to a full memory fence (e.g. `mfence`/`dmb ish`) on
  the store. That extra global ordering buys nothing here — the queue only needs
  the pairwise producer↔consumer handshake — and it costs latency on the hot
  path.

Acquire/release is the **minimal ordering that is still correct**: exactly the
producer-writes-then-consumer-reads handshake, and nothing more. That is why it
is the standard idiom for SPSC ring buffers and why this design uses it.

### Why the earlier design needed completing

The original sketch had *both* counters written by the producer, so the
consumer's progress was never reported back to the producer. Without that
feedback the producer could not know when it was safe to wrap and would silently
overwrite unread data (message loss). The completed design flips ownership —
**producer owns the head, consumer owns the tail** — which is exactly what makes
the loss-free back-pressure in §4 possible.

---

## 7. Worked example of wrap-around

`QUEUE_SIZE = 1024`. Suppose the head is at logical offset `1000`
(`1000 & 1023 = 1000`) and we write a 40-byte record (4-byte header + 36-byte
payload):

- `ring_write` for the header: `index = 1000`, `first = min(4, 1024-1000=24) = 4`
  → header written at bytes `1000..1003`, no wrap.
- `ring_write` for the payload: `counter = 1004`, `index = 1004`,
  `first = min(36, 1024-1004=20) = 20` → 20 bytes at `1004..1023`, then the
  remaining `16` bytes wrap to `0..15`.
- Head advances to `1040`; next record starts at `1040 & 1023 = 16`.

The consumer, reading from tail `1000`, performs the identical split in
`ring_read` and reassembles the message contiguously in `out`. The absolute
counters (`1000 → 1040`) never wrap; only their masked *indices* do.

---

## 8. The test suite (`fast_queue::test()`)

All tests and benchmarks live in `fast_queue_SPSC_test.hpp` (see *Source layout*
above) and run from `fast_queue::test()`. There are two correctness demos and two
benchmark families. Throughput benchmarks use [Google Benchmark]; the wait
strategy is a compile-time flag (`BusySpin`) so each benchmark exists in a
**busy-spin** and a **yield** variant for direct comparison (see §9).

### `test_basic` — single-message round-trip
A single `Quote` struct is written and read back, confirming the round-trip and
that the queue reports empty afterward.

### `test_limits` — the limit check, deterministically
Fixed 8-byte records (`4` header + `4` payload) are written until `try_write`
returns `false`. With `QUEUE_SIZE = 1024` exactly `1024 / 8 = 128` records fit;
the test asserts precisely `128` were accepted, that the `129th` is **rejected**
(the counters correctly report *full*), and that draining returns all `128`
values **in order**. This is the direct proof that counters are checked against
the limit.

### `run_full_ring` — concurrent stress + throughput
Two threads pump **1,000,000 variable-sized messages** (payloads of 8–44 bytes,
sizes chosen with `seq % 37` so records wrap at *unaligned* offsets and exercise
the split-`memcpy` path). Each payload embeds its sequence number plus a
deterministic filler pattern.

- The **producer** spins on back-pressure until there is room, so it is
  repeatedly driven into the *full* state (the benchmark prints how many times).
- The **consumer** asserts every message arrives with the **expected sequence
  number** (in order, none skipped, none duplicated) and that every filler byte
  matches (byte-for-byte integrity).

The driver is instantiated **four** ways — two ring sizes × two wait strategies:

| Benchmark | Ring | Wait strategy |
|-----------|------|---------------|
| `test_full_ring_back_pressure` | 1 KB (small) | busy-spin |
| `test_full_ring_back_pressure_yield` | 1 KB (small) | `std::this_thread::yield()` |
| `test_full_ring_optimized` | 1 MB (large) | busy-spin |
| `test_full_ring_optimized_yield` | 1 MB (large) | `std::this_thread::yield()` |

The **small ring** keeps producer and consumer colliding (constantly full/empty),
isolating back-pressure/contention cost; the 1 MB of traffic through 1 KB wraps
the buffer tens of thousands of times. The **large ring** lets them decouple so
full/empty stalls nearly vanish and throughput reflects raw data-movement cost.
Timing is *manual* and brackets only the pump — thread spawn/join and payload
construction are excluded.

### `run_latency` — end-to-end delivery latency
Models an HFT feed: messages arrive at a target **rate** (100 K and 1 M msg/s)
with real gaps between them — nobody sleeps. The producer **busy-waits on the
clock** until each message's scheduled send time (like a feed handler polling a
NIC), stamps it with a publish timestamp, and enqueues it; the consumer stamps
arrival the instant it dequeues, so `recv - t_send` is true end-to-end latency.

Every per-message sample is kept and, after the run, sorted to report
**p50 / p99 / p99.9** alongside avg/min/max — because the mean hides the tail and
`max` is a single noisy sample, while the **tail percentiles are what actually
characterize HFT latency** (you get picked off on your worst cases, not your
average). Two variants, `test_latency` (busy-spin) and `test_latency_yield`,
select the consumer's wait strategy during the idle gaps between messages.

Verified with a clean `-Wall -Wextra` build, all assertions passing, and clean
under `-fsanitize=thread` (see §6).

[Google Benchmark]: https://github.com/google/benchmark

---

## 9. Performance measurement

> **⚠️ Read these as relative comparisons, not absolute specs.** Numbers below are
> from a **development laptop** (Apple Silicon, 10 cores), **no core pinning**
> (the run reports *"Failed to set thread affinity"*), under normal background
> load (load average ≈ 1.8). There is no dedicated/isolated core, no NUMA control,
> and the OS is free to migrate and deschedule the threads. Absolute latencies —
> especially the tail — would be markedly lower and more stable on a tuned server
> with isolated, pinned cores. What *is* meaningful here is the **A/B contrast**
> between wait strategies and ring sizes, measured on the same machine back to back.

### Throughput — `run_full_ring` (1,000,000 messages/iteration, 10 iterations)

| Benchmark | Ring | Wait | Throughput | Producer hit "full" |
|-----------|------|------|-----------:|--------------------:|
| `test_full_ring_back_pressure` | 1 KB | busy-spin | ~48 M msg/s | 676,676 |
| `test_full_ring_back_pressure_yield` | 1 KB | yield | ~52 M msg/s | 18,432 |
| `test_full_ring_optimized` | 1 MB | busy-spin | ~64 M msg/s | 1,043,304 |
| `test_full_ring_optimized_yield` | 1 MB | yield | ~77 M msg/s | 19,640 |

**Interpretation:**

- **Bigger ring → more throughput** (48→64, 52→77 M msg/s). Decoupling the two
  threads removes back-pressure stalls; throughput moves toward the raw
  data-movement cost of the queue rather than the cost of waiting on each other.
- **Under back-pressure, `yield` edges out busy-spin on throughput** (52 vs 48,
  77 vs 64). This looks backwards until you read the "hit full" column: busy-spin
  hammers `try_write` hundreds of thousands / a million times while spinning on a
  full ring, burning a core to *poll* space that isn't there. `yield` deschedules
  instead, so the consumer gets the core and drains faster — fewer full events by
  ~35×. When one side is chronically blocked, giving up the core beats spinning.
- **Throughput is the wrong metric to pick the strategy on**, though — see below.

### Latency — `run_latency` (50 K samples × 3 iterations = 150 K/rate)

| Rate | Strategy | p50 | p99 | p99.9 | max |
|------|----------|----:|----:|------:|----:|
| 1 M msg/s | busy-spin | 125 ns | **166 ns** | 8.3 µs | 36 µs |
| 1 M msg/s | yield | 125 ns | **14.0 µs** | 78 µs | 114 µs |
| 100 K msg/s | busy-spin | 125 ns | **167 ns** | 7.2 µs | 87 µs |
| 100 K msg/s | yield | 125 ns | **9.6 µs** | 72 µs | 162 µs |

**Interpretation — this is where busy-spin earns its keep:**

- **The median is identical (125 ns) for both strategies.** Mean and median
  completely hide the difference — exactly why we report percentiles.
- **The tail is where they diverge, by ~85×.** Busy-spin holds p99 at ~166 ns;
  yield's p99 blows up to **9–14 µs**. When the consumer yields during the idle
  gap between messages, the OS deschedules it, and the *next* message waits for a
  reschedule — a multi-µs stall that lands squarely in the tail. Busy-spin keeps
  the consumer hot so it notices the next message within tens of ns.
- **This is the whole reason HFT burns a core.** You accept lower throughput
  under contention (previous table) and 100 % CPU on an idle queue in exchange
  for a tight, predictable tail — because in trading you get picked off on your
  p99/p99.9, not your average. The two tables together are the trade-off stated
  numerically: **yield wins throughput-under-contention; busy-spin wins the
  latency tail — and the tail is what matters.**
- **`max` is visibly the noisy outlier the percentiles replace** (36–162 µs,
  jumping around run to run on an unpinned laptop), which is exactly why p99.9 is
  the honest tail number to quote.

### Reproducing

```
cmake --build build_release
./build_release/low_latency                       # runs everything
./build_release/low_latency --benchmark_filter='test_latency'   # just latency
```

---

## 10. Properties at a glance

| Property                 | How it's achieved                                             |
|--------------------------|---------------------------------------------------------------|
| Lock-free                | two atomic counters, acquire/release only; no mutex, no CAS   |
| No message loss          | producer back-pressure via the free-space limit check         |
| In-order, complete       | FIFO byte stream + length-prefixed framing                    |
| Full capacity usable     | empty/full distinguished by counter *difference*              |
| Circular storage         | power-of-two size + `& mask` + split-`memcpy` wrap helpers     |
| No false sharing         | each counter cache-line aligned                               |
| Cheap hot path           | cached counter snapshots; atomic touched only when needed     |
| Variable-sized messages  | `int32` length prefix per record                              |

### Constraints / assumptions

- **Exactly one** producer thread and **one** consumer thread (SPSC). Multiple
  producers or consumers would need additional synchronization.
- A single message must fit in the buffer: `sizeof(header_t) + payload ≤ QUEUE_SIZE`
  (asserted).
- The ring `Size` must be a power of two (static-asserted in `fast_queue_t`).
- Payloads are raw bytes. The queue itself only moves bytes; the `to_bytes` /
  `from_bytes` helpers (which require trivially copyable types) used to
  (de)serialize structs like `Quote` are **test-side conveniences** and live in
  `fast_queue_SPSC_test.hpp`, not in the implementation header.
