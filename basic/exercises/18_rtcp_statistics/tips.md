# Day 18 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Guard `expected == 0` before any division

Loss is `(expected - received) / expected` only when `expected > 0` and
`received < expected`. Zero expected packets — even if `received` is
nonzero — must yield `0.0` with no divide. Surplus received also yields
`0.0` so the fraction never goes negative.

## 2. Jitter is an input, not a computation

Copy `input.jitter` into the report. RFC 3550 A.8 interarrival jitter
needs arrival times this API does not have. Do not call `now()` or
sleep.

## 3. LSR/DLSR without `A` cannot make RTT

A real RTT sample is `A - LSR - DLSR`. `ReceiverInputs` has `lsr` and
`dlsr` but no arrival NTP timestamp `A`. `lsr == 0` or `dlsr == 0` is
the protocol “no sample” case. With no `A`, both-nonzero is still no
estimate. Set `round_trip` to `std::nullopt` always.

## 4. Stateless value function

`make_report` must not remember the last call. Each report is computed
from that one `ReceiverInputs`. Self-check: `(10, 8)` → loss `0.2` and
the given jitter; `(0, *)` → loss `0`; any LSR/DLSR → no RTT.

## Relevant knowledge

RTCP Receiver Reports carry fraction lost, interarrival jitter, LSR,
and DLSR (RFC 3550 §6.4.1). Fraction lost is an 8-bit field on the
wire; this lab uses a `double` in `[0, 1]` for teaching. Jitter and RTT
are separate: jitter is copied; RTT is omitted because the arrival time
is not an input.

C++ technique: convert the subtraction to `double` only after the zero
and surplus guards. `std::optional<double>` for “no RTT”. No sockets,
no chrono waits.

Day 22 (next task) snapshots atomic packet/byte counters. Those reads
are **per-counter atomic**, not a pair-consistent `(packets, bytes)`
unless that task adds a lock. Do not implement Day 22 here.

## Exercise contract

- **Value API:** no class state. Preserve the existing structs and
  `make_report(const ReceiverInputs &) → ReceiverReport`.
- **Loss:** `0.0` if `expected == 0` or `received >= expected`; else
  `(expected - received) / double(expected)`.
- **Jitter:** copied unchanged.
- **RTT:** always `nullopt` (no `A`; also when `lsr == 0` or
  `dlsr == 0`).
- Offline and deterministic.

## Solution steps

1. Copy jitter.
2. Set `round_trip` empty.
3. Branch on `expected == 0` and `received >= expected` before dividing.
4. Otherwise compute the `0.2`-style fraction.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| expected 10, received 8, jitter 4.5 | loss 0.2, jitter 4.5, no RTT |
| expected 0, received 0 | loss 0.0 |
| expected 0, received 3 | loss 0.0, no division |
| expected 5, received 7 | loss 0.0 |
| expected 10, received 10 | loss 0.0 |
| lsr/dlsr both 0 | no RTT |
| lsr/dlsr both nonzero | still no RTT |

Common mistakes: dividing by `expected` when it is 0; returning
`dlsr / 65536.0` as a fake RTT; mutating a static last-report;
recomputing jitter from imaginary arrivals.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Avoid division by zero and document the no-RTT case.
- Preserve jitter and compute loss from the documented input fields only.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstdint>
#include <optional>
namespace practice::day18 {

struct ReceiverInputs {
  std::uint32_t expected{};
  std::uint32_t received{};
  double jitter{};
  std::uint32_t lsr{};
  std::uint32_t dlsr{};
};

struct ReceiverReport {
  double loss_fraction{};
  double jitter{};
  std::optional<double> round_trip;
};

[[nodiscard]] ReceiverReport make_report(const ReceiverInputs &input);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day18
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day18 {

ReceiverReport make_report(const ReceiverInputs &input) {
  ReceiverReport report;
  report.jitter = input.jitter;
  report.round_trip = std::nullopt;
  if (input.expected == 0 || input.received >= input.expected) {
    report.loss_fraction = 0.0;
  } else {
    report.loss_fraction =
        static_cast<double>(input.expected - input.received) /
        static_cast<double>(input.expected);
  }
  return report;
}

int Solution::run(std::ostream &out) {
  out << "--- day 18 ---\n";
  int ret = 0;

  const auto lost = make_report({10, 8, 4.5, 0, 0});
  ret += is_ok(lost.loss_fraction == 0.2);
  ret += is_ok(lost.jitter == 4.5);
  ret += is_ok(!lost.round_trip.has_value());

  const auto zero = make_report({0, 0, 1.0, 0, 0});
  ret += is_ok(zero.loss_fraction == 0.0);
  ret += is_ok(zero.jitter == 1.0);
  ret += is_ok(!zero.round_trip.has_value());

  const auto surplus = make_report({5, 7, 0.0, 1, 2});
  ret += is_ok(surplus.loss_fraction == 0.0);
  ret += is_ok(!surplus.round_trip.has_value());

  const auto none_received = make_report({4, 0, 2.0, 9, 9});
  ret += is_ok(none_received.loss_fraction == 1.0);
  ret += is_ok(none_received.jitter == 2.0);
  ret += is_ok(!none_received.round_trip.has_value());
  return ret;
}

} // namespace practice::day18
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day18
./build/basic/cpp_rtc_practice --run 18
```

`run()` must return 0. Loss is 0.2 for 10/8. Zero expected never
divides. Jitter is unchanged. `round_trip` stays empty. Do not sample
the clock or open a socket.
