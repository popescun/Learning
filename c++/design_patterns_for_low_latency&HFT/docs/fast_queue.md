# Fast Queue — a lock-free SPSC ring buffer

`fast_queue.hpp` implements a **single-producer / single-consumer (SPSC)**,
lock-free, byte-oriented **ring buffer** designed for low-latency / HFT-style
message passing between two threads (e.g. a market-data feed thread and a
strategy thread).

It carries **variable-sized, length-prefixed messages**, reuses its storage
**circularly**, applies **back-pressure** so no message is ever lost, and
synchronizes the two threads with nothing more than two atomic counters and
acquire/release ordering — no mutexes, no CAS loops.

---

## 1. Data model

```
struct fast_queue {
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> read_counter{0};
  alignas(CACHE_LINE_SIZE) std::atomic<std::uint64_t> write_counter{0};
  alignas(CACHE_LINE_SIZE) std::array<std::byte, QUEUE_SIZE> buffer{};
};
```

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
entire `QUEUE_SIZE` bytes are usable. (Classic pointer-based ring buffers have
to leave one slot empty to tell the two states apart; this design does not.)

#### Why classic ring buffers waste a slot

A textbook ring buffer keeps two indices that point *into* the buffer, each
wrapping via modulo:

```
head = write position   (0 .. N-1)
tail = read position    (0 .. N-1)
```

- **Empty** is naturally `head == tail` (the reader has caught up to the writer).
- **Full** is *also* `head == tail`: writing enough to wrap the head all the way
  back around brings the two indices together again.

So `head == tail` means *both* empty and full, and you cannot tell which. The
indices live in `[0, N)` — only `N` distinct values — but the fill level has
`N + 1` possible states (0, 1, … N bytes). `N` slots cannot encode `N + 1`
states, so one state is unrepresentable.

The usual fix is to **sacrifice one slot**: declare the buffer full when
advancing the head *would* collide with the tail, i.e. `(head + 1) % N == tail`.
Now the two states are distinguishable, but one slot is never used:

```
N = 8, one slot always kept empty:
[ A ][ B ][ C ][ D ][ E ][ F ][ G ][   ]   <- "full", slot 7 unused
                                     ^ tail
                                 ^ head + 1 == tail
```

A 1024-byte buffer effectively holds 1023.

#### Why this design uses the whole buffer

Here the counters are **monotonically increasing absolute byte totals**, not
positions in `[0, N)`. The state is derived from their difference, which lives in
`[0, N]` — that is `N + 1` distinct values, exactly enough to represent every
fill level:

```
bytes_available_to_read = write_counter - read_counter   // in [0, QUEUE_SIZE]

empty ⇔ difference == 0
full  ⇔ difference == QUEUE_SIZE
```

Because `0` and `N` are different numbers, empty and full are never confused —
even though both map to the *same physical offset* (`head & MASK == tail & MASK`).
The wrap that made the classic scheme ambiguous is invisible to the bookkeeping:
the counters keep climbing past `N`, `2N`, `3N`… and are only masked down when
memory is actually touched (see *Mapping a counter to a buffer position — a
bit-mask, not a modulo* below).
All `QUEUE_SIZE` bytes are usable — no sacrificed slot.

The trade this makes is relying on the counters not overflowing. With
`uint64_t` byte counters, even at 10 GB/s that takes ~58 years, which is why we
never worry about counter wrap. Classic 32-bit position indices dodge overflow
but pay the empty-slot tax instead; for a 64-bit queue the difference-of-counters
trade is clearly the better one.

### Mapping a counter to a buffer position — a bit-mask, not a modulo

**The offset is `counter % QUEUE_SIZE`, but we compute it as `counter &
QUEUE_MASK` — a single-instruction bit-mask instead of an expensive `%`
divide.** The two are identical because `QUEUE_SIZE` is a power of two.

```
offset_in_buffer = counter & QUEUE_MASK           // == counter % QUEUE_SIZE
                                                  // QUEUE_MASK = QUEUE_SIZE - 1
```

Why they're equal: dividing by `2^k` is a shift right by `k`, and the remainder
is exactly the `k` low bits that get shifted off. `& QUEUE_MASK` keeps precisely
those low bits, so it *is* the modulo — for a power-of-two divisor only.

This is why the equivalence holds only when `QUEUE_SIZE` is a **power of two**,
which is enforced at compile time (the assert uses the same trick: a power of two
has one bit set, so `N & (N - 1) == 0`):

```
constexpr std::size_t QUEUE_SIZE = 1024;
constexpr std::uint64_t QUEUE_MASK = QUEUE_SIZE - 1;
static_assert((QUEUE_SIZE & QUEUE_MASK) == 0, "must be a power of two");
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
inline void ring_write(fast_queue &fq, std::uint64_t pos,
                       const std::byte *src, std::size_t n) {
  const std::size_t offset = pos & QUEUE_MASK;
  const std::size_t first  = std::min(n, QUEUE_SIZE - offset); // bytes until end
  std::memcpy(fq.buffer.data() + offset, src, first);          // part 1
  if (n > first)                                                // wrapped?
    std::memcpy(fq.buffer.data(), src + first, n - first);     // part 2 @ start
}
```

`ring_read` is the mirror image (buffer → destination). `first` is how many
bytes fit before the physical end; if `n > first` the remainder wraps to offset
`0`. When a record fits without wrapping, the second `memcpy` is skipped.

This split logic applies to **both** the 4-byte header and the payload, so even
the length prefix itself may straddle the boundary and is handled correctly.

---

## 4. The producer — `try_write`

```cpp
bool try_write(fast_queue &fq, std::span<const std::byte> payload);
```

Returns `true` if the message was written, `false` if the queue was too full to
hold the whole record (nothing is written in that case — messages are never
partially enqueued).

The producer keeps two **private** (non-atomic) variables:

```cpp
std::uint64_t write_counter{0}; // its own copy of the head
std::uint64_t read_counter{0};  // last value it observed of the tail
```

### Step 1 — the limit check (back-pressure)

```cpp
std::uint64_t bytes_available_to_read = write_counter - read_counter;
if (bytes_available_to_read + record_size > QUEUE_SIZE) {         // maybe full?
  read_counter = fq.read_counter.load(std::memory_order_acquire); // refresh
  bytes_available_to_read = write_counter - read_counter;
  if (bytes_available_to_read + record_size > QUEUE_SIZE)
    return false;                                                 // truly full
}
```

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
const auto len = static_cast<header_t>(payload.size());
ring_write(fq, write_counter,             &len,          sizeof(len)); // header
ring_write(fq, write_counter + sizeof(len), payload.data(), payload.size());
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
std::optional<std::size_t> try_read(fast_queue &fq, std::span<std::byte> out);
```

Returns the payload length that was copied into `out`, or `std::nullopt` if the
queue was empty. The consumer keeps its own private mirror:

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
header_t len{};
ring_read(fq, read_counter, &len, sizeof(len));            // 1. read length
// assert len fits in `out`
ring_read(fq, read_counter + sizeof(len), out.data(), len); // 2. read payload
```

The header is read first to learn the payload length, then exactly that many
payload bytes are copied out — correctly reconstructing a variable-sized message.
Because the producer published `write_counter` only *after* fully writing the
record, and the consumer never reads beyond `write_counter`, a complete record is
always present before it is read.

### Step 3 — publish the freed space

```cpp
read_counter += sizeof(len) + len;                            // advance tail
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

- `ring_write` for the header: `offset = 1000`, `first = min(4, 1024-1000=24) = 4`
  → header written at bytes `1000..1003`, no wrap.
- `ring_write` for the payload: `pos = 1004`, `offset = 1004`,
  `first = min(36, 1024-1004=20) = 20` → 20 bytes at `1004..1023`, then the
  remaining `16` bytes wrap to `0..15`.
- Head advances to `1040`; next record starts at `1040 & 1023 = 16`.

The consumer, reading from tail `1000`, performs the identical split in
`ring_read` and reassembles the message contiguously in `out`. The logical
counters (`1000 → 1040`) never wrap; only their masked *positions* do.

---

## 8. The test suite (`fast_queue::test()`)

Three demos exercise the implementation end to end:

### `test_basic`
A single `Quote` struct is written and read back, confirming the round-trip and
that the queue reports empty afterward.

### `test_limits` — the limit check, deterministically
Fixed 8-byte records (`4` header + `4` payload) are written until `try_write`
returns `false`. With `QUEUE_SIZE = 1024` exactly `1024 / 8 = 128` records fit;
the test asserts precisely `128` were accepted, that the `129th` is **rejected**
(the counters correctly report *full*), and that draining returns all `128`
values **in order**. This is the direct proof that counters are checked against
the limit.

### `test_full_ring` — full concurrent stress, no loss
Two threads pump **1,000,000 variable-sized messages** (payloads of 8–44 bytes,
sizes chosen with `seq % 37` so records wrap at *unaligned* offsets and exercise
the split-`memcpy` path). Each payload embeds its sequence number plus a
deterministic filler pattern.

- The **producer** spins on back-pressure — `while (!try_write(...)) yield();` —
  so it is repeatedly driven into the *full* state (the test prints how many
  times this happened, typically thousands).
- The **consumer** asserts every message arrives with the **expected sequence
  number** (in order, none skipped, none duplicated) and that every filler byte
  matches (byte-for-byte integrity).

Because the 1 MB of traffic flows through a 1 KB ring, the buffer wraps around
tens of thousands of times. Passing this test demonstrates the three guarantees
asked for: **complete** delivery, **in-order** delivery, and **no loss**.

Run it via `fast_queue::test()` from `main`. It was verified with a clean
`-Wall -Wextra` build, all assertions passing, and clean under `-fsanitize=thread`.

---

## 9. Properties at a glance

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
- `QUEUE_SIZE` must be a power of two (static-asserted).
- Payloads are raw bytes; use `to_bytes` / `from_bytes` (which require trivially
  copyable types) to (de)serialize structs like `Quote`.
