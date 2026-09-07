# Day 22 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Two atomics, not a locked pair

Store `packets_` and `bytes_` as `std::atomic<std::uint64_t>`, both
initialized to `0`. `add_packet` and `snapshot` stay exactly as declared.
Do not add a mutex unless you also change the documented guarantee —
this exercise does not.

## 2. `add_packet` updates both counters

`fetch_add(1)` on packets, `fetch_add(bytes)` on the byte counter.
A zero-byte packet still increments the packet count. There is no
failure return.

## 3. `snapshot` is two independent loads

Load packets, load bytes, return `{packets, bytes}`. Each load is
atomic. The pair is **not**. A concurrent writer can slip one
`fetch_add` between those loads. Do not claim seq_cst makes the pair
atomic.

## 4. Join, then assert exact totals

Use `std::jthread` so the destructor joins. After the writer block
ends, `snapshot()` must equal the planned `(packets, bytes)`. While
writers are still running, only check per-counter bounds. No
`sleep_for`.

## Relevant knowledge

RTP senders and receivers often keep packet and octet counts for RTCP
Sender/Receiver Reports (RFC 3550). This lab is only the counter
object: no wire format, no RTCP, no arrival timestamps.

C++ technique: `std::atomic<std::uint64_t>` `fetch_add` / `load` with
`memory_order_relaxed`. Two 64-bit atomics cannot be loaded as one
portable pair without a lock, a seqlock, or a platform 128-bit atomic.
`std::jthread` is the RAII join path. Bounded `for` loops keep the
self-check offline and deterministic. No sockets, no chrono waits.

## Exercise contract

- **State:** private `std::atomic<std::uint64_t> packets_{0}` and
  `bytes_{0}`. Public API unchanged:
  `add_packet(std::uint64_t)` and `snapshot() const → StatsSnapshot`.
- **`add_packet(n)`:** atomically increment packets by 1 and bytes by
  `n` (`n` may be 0).
- **`snapshot()`:** each field is one atomic load. **Per-counter
  atomic, not pair-consistent.** A live pair may disagree. After all
  writers join, both fields match the exact totals.
- Counters are monotonic `uint64_t` from 0, so a snapshot cannot show
  a signed negative-like underflow.
- Worker loops are bounded. Threads are RAII (`std::jthread`).
- Offline and deterministic: no sleep, no sockets, no wall-clock wait.

## Solution steps

1. Declare the two private atomics (already in `starter.hpp`).
2. Implement `add_packet` with two `fetch_add` calls (`relaxed`).
3. Implement `snapshot` with two `load` calls (`relaxed`).
4. In `run()`, add sequential totals, then bounded writers whose
   destructors join before the exact-total check, plus a bounded
   reader that only checks per-counter bounds.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| default snapshot | `(0, 0)` |
| `add_packet(160)` once | `(1, 160)` |
| `add_packet(0)` | packets +1, bytes unchanged |
| `160`, then `0`, then `320` | `(3, 480)` |
| 4 writers × 250 packets × 160 bytes, after join | `(1000, 160000)` |
| snapshot during those writes | each field `<=` its planned max; pair may disagree |
| `seq_cst` on each op | still not a pair-consistent snapshot |

Common mistakes: using plain `uint64_t` without atomics; asserting
`bytes == packets * size` on a live snapshot; sleeping to “wait” for
writers; detaching threads; claiming the pair is atomic because each
field is; using signed counters that can look negative.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Use atomics with a documented snapshot consistency guarantee.
- Keep worker loops bounded and use RAII-managed threads.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <atomic>
#include <cstdint>
namespace practice::day22 {
struct StatsSnapshot {
  std::uint64_t packets{};
  std::uint64_t bytes{};
};
class RtpStats {
public:
  void add_packet(std::uint64_t bytes);
  [[nodiscard]] StatsSnapshot snapshot() const;

private:
  std::atomic<std::uint64_t> packets_{0};
  std::atomic<std::uint64_t> bytes_{0};
};
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day22
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <thread>
#include <vector>

namespace practice::day22 {

void RtpStats::add_packet(std::uint64_t bytes) {
  packets_.fetch_add(1, std::memory_order_relaxed);
  bytes_.fetch_add(bytes, std::memory_order_relaxed);
}

StatsSnapshot RtpStats::snapshot() const {
  return StatsSnapshot{
      packets_.load(std::memory_order_relaxed),
      bytes_.load(std::memory_order_relaxed),
  };
}

int Solution::run(std::ostream &out) {
  out << "--- day 22 ---\n";
  int ret = 0;

  {
    RtpStats stats;
    const auto empty = stats.snapshot();
    ret += is_ok(empty.packets == 0);
    ret += is_ok(empty.bytes == 0);
    stats.add_packet(160);
    stats.add_packet(0);
    stats.add_packet(320);
    const auto snap = stats.snapshot();
    ret += is_ok(snap.packets == 3);
    ret += is_ok(snap.bytes == 480);
  }

  {
    RtpStats stats;
    constexpr std::uint64_t kWriters = 4;
    constexpr std::uint64_t kPacketsPerWriter = 250;
    constexpr std::uint64_t kBytesPerPacket = 160;
    constexpr std::uint64_t kPackets = kWriters * kPacketsPerWriter;
    constexpr std::uint64_t kBytes = kPackets * kBytesPerPacket;
    {
      std::vector<std::jthread> writers;
      writers.reserve(kWriters);
      for (std::uint64_t w = 0; w < kWriters; ++w) {
        writers.emplace_back([&stats] {
          for (std::uint64_t i = 0; i < kPacketsPerWriter; ++i) {
            stats.add_packet(kBytesPerPacket);
          }
        });
      }
    }
    const auto snap = stats.snapshot();
    ret += is_ok(snap.packets == kPackets);
    ret += is_ok(snap.bytes == kBytes);
  }

  {
    RtpStats stats;
    constexpr std::uint64_t kWriters = 4;
    constexpr std::uint64_t kPacketsPerWriter = 250;
    constexpr std::uint64_t kBytesPerPacket = 160;
    constexpr std::uint64_t kMaxPackets = kWriters * kPacketsPerWriter;
    constexpr std::uint64_t kMaxBytes = kMaxPackets * kBytesPerPacket;
    std::atomic<bool> reader_ok{true};
    {
      std::vector<std::jthread> workers;
      workers.reserve(kWriters + 1);
      for (std::uint64_t w = 0; w < kWriters; ++w) {
        workers.emplace_back([&stats] {
          for (std::uint64_t i = 0; i < kPacketsPerWriter; ++i) {
            stats.add_packet(kBytesPerPacket);
          }
        });
      }
      workers.emplace_back([&] {
        for (int i = 0; i < 4000; ++i) {
          const auto snap = stats.snapshot();
          if (snap.packets > kMaxPackets || snap.bytes > kMaxBytes) {
            reader_ok.store(false, std::memory_order_relaxed);
          }
        }
      });
    }
    ret += is_ok(reader_ok.load(std::memory_order_relaxed));
    const auto snap = stats.snapshot();
    ret += is_ok(snap.packets == kMaxPackets);
    ret += is_ok(snap.bytes == kMaxBytes);
  }
  return ret;
}

} // namespace practice::day22
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day22
./build/basic/cpp_rtc_practice --run 22
```

`run()` must return 0. Sequential mixed sizes are `(3, 480)`. After
four writers of 250×160 join, the snapshot is `(1000, 160000)`. A
reader that runs during those writes may see a torn pair; it must
still see each field within `[0, planned max]`. Use atomics and
`jthread`; do not sleep or open a socket. Do not require
`bytes == packets * size` on a live snapshot.
