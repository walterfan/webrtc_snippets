# Day 22 — Atomic RTP Statistics

Implement `RtpStats`. Fixed writers add packet/byte counts while readers
obtain snapshots. A snapshot must never contain impossible negative-like
values; document its consistency guarantee. Use bounded worker loops and
RAII threads. Focus: atomics and memory ordering.

## Requirements

### Storage

`RtpStats` owns two private counters:

- `std::atomic<std::uint64_t> packets_{0}`
- `std::atomic<std::uint64_t> bytes_{0}`

Initialize both to zero. Do not add public fields. Do not change the
public signatures of `add_packet(std::uint64_t)` or `snapshot()`.

### `add_packet`

`add_packet(n)` updates **both** counters: increment `packets_` by one
and add `n` to `bytes_`. `n` may be zero (a zero-byte packet still
counts as one packet). Use atomic read-modify-write (`fetch_add`).
There is no reject path.

### Snapshot guarantee — per-counter atomic, not pair-consistent

`snapshot()` loads each counter with an atomic `load`. Each field is
therefore a value that some prefix of `add_packet` calls produced for
**that** counter.

The pair `(packets, bytes)` is **not** one atomic observation. A
concurrent `add_packet` may complete one `fetch_add` between the two
loads. A live snapshot may therefore show a packet count that does not
match the byte count (for example `packets == 3` and `bytes` still at
the total for two packets). `memory_order_seq_cst` on each operation
does **not** make the pair atomic. A lock, a seqlock, or a single
packed wide atomic would; this API has none of those.

Do **not** assert `bytes == packets * packet_size` on a snapshot taken
while writers are still running. After every writer `jthread` has been
joined, both counters must match the exact planned totals.

Each counter is monotonic: it starts at 0 and only increments, so a
snapshot cannot contain a signed “negative-like” underflow. During
bounded writers, each observed field is at most the planned maximum
for that field.

`memory_order_relaxed` is enough for this teaching counter pair because
the claimed guarantee is per-counter atomicity, not a happens-before
between the two fields. Writer-thread destruction (join) is the
synchronization used for the final-total check.

### Workers

Keep writer and reader loops **bounded** (fixed iteration counts). Use
RAII-managed threads (`std::jthread`). Do not `sleep_for`, do not wait
on a wall clock, and do not open sockets.

## How to start

1. Read the private atomics in `starter.hpp`. Keep the public
   `add_packet` / `snapshot` signatures unchanged.
2. Implement the two methods in `starter.cpp`; keep `Solution::run` as
   a render helper until the logic is ready.
3. Use `tips.md` if you stall; do not copy a finished solution from
   elsewhere.
4. Run `ctest --test-dir build/basic -R Day22` after wiring a real
   check.

## Acceptance fixtures

### Happy path — sequential totals

- **Given** an empty `RtpStats`
- **When** `add_packet(160)`, `add_packet(0)`, then `add_packet(320)`
- **Then** `snapshot()` is `(3, 480)`

### Happy path — concurrent writers, exact final total

- **Given** four `jthread` writers, each adding 250 packets of 160
  bytes
- **When** all writers have joined
- **Then** `snapshot()` is `(1000, 160000)`

### Concurrent read — per-counter bounds only

- **Given** the same bounded writers plus one bounded reader loop
- **When** the reader calls `snapshot()` during the writes
- **Then** each field is `<=` its planned maximum and never looks
  wrapped/negative; the reader must **not** require the pair to match

### Empty

- **Given** a default-constructed `RtpStats`
- **When** `snapshot()` runs before any `add_packet`
- **Then** the result is `(0, 0)`
