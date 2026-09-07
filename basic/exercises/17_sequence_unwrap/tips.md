# Day 17 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Three pieces of state, not a running sum of raw seq

Store “have I seen a packet”, the last **16-bit** sequence, and the last
**64-bit** extended value. The first `unwrap` copies the 16-bit input
into both last-sequence and last-extended. Later calls compute a signed
step from the last 16-bit value, then apply that step to the extended
counter.

## 2. Promote-then-cast; the subtract itself is not 16-bit modular

`uint16_t` operands promote to `int` before subtraction, so
`0 - 65535` is the signed `int` `-65535`. Cast that difference to
`std::uint16_t` first (mod 65536 → `1`), then to `std::int16_t` for
the half-range step in `[-32768, 32767]`. `65535` then `0` becomes
`delta == 1`. Do not describe the raw subtract as already modular.

## 3. Bound reorders to 64; farther back is a wrap

If `delta` is in `[-64, -1]` **and** `last_extended_ >= -delta`, treat
it as reorder and subtract. If `delta < -64`, add `delta + 65536` so a
jump such as `100` then `20` is a forward wrap (`65556`), not an
80-behind reorder. `kReorderBound` is 64.

## 4. Never widen a negative `delta` through `uint64_t`

`uint64_t(-2)` is a huge increment. Add when `delta >= 0`; subtract
`-delta` when `delta` is a 64-window reorder that stays non-negative.
If that subtract would drop the extended value below 0 — the floor
case `0` then `65535` (`delta == -1`) — use the forward-wrap step
instead (`0, 65535` → `0, 65535`). Self-check: `65535, 0` →
`65535, 65536`; `10, 8, 9` → `10, 8, 9`; `100, 20` → `100, 65556`.

## Relevant knowledge

RTP sequence numbers are 16-bit and wrap. Receivers keep an extended
sequence (RFC 3550 Appendix A.1 style) so loss and jitter math can use
a 32/64-bit counter. Half-range signed difference is the usual wrap
test: distances larger than 32768 the other way are wraps, not huge
jumps.

This lab tightens the backward side to **64** so a large reverse jump
is classified as the next cycle, matching a small reorder window rather
than the full 32768-wide half space.

C++ technique: `int` promotion, `uint16_t` wrap, `int16_t`
reinterpretation, then checked apply onto `uint64_t`. No sockets, no
sleep.

## Exercise contract

- **State:** `initialized_`, `last_sequence_`, `last_extended_`.
- **First call:** all three become true / `s` / `s`; return `s`.
- **`delta`:** `int16_t` of the uint16 difference from last sequence.
- **`delta >= 0`:** add to extended (0 is a no-op increment).
- **`[-64, -1]`:** subtract if `last_extended_ >= -delta`; if
  `last_extended_ < -delta` (floor / underflow), forward-wrap.
- **`< -64`:** forward wrap, add `delta + 65536`.
- Then store last sequence and last extended. Deterministic. Do not
  treat the promoted `int` subtract as already modular.

## Solution steps

1. Handle the uninitialized first packet.
2. Compute `delta` with uint16 subtract + `int16_t` cast.
3. Branch forward / in-window reorder / far-back wrap.
4. Apply the step with add or subtract; update both last fields.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| first value `7` | `7`, now initialized |
| `65535` then `0` | `65535`, `65536` |
| `10` then `8` then `9` | `10`, `8`, `9` |
| `100` then `20` | `100`, `65556` (80 > 64) |
| same seq twice | same extended twice |
| `0` then `65535` | would-be `-1` underflows; forward wrap `65535` |

Common mistakes: signed `int` subtract of 16-bit values; adding
`uint64_t(negative)`; using a 32768 reorder window; forgetting to
update `last_sequence_` on reorder.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Define one rollover and bounded reordering policy using fixed-width unsigned
values.
- Avoid signed overflow and keep resulting extended values deterministic.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstdint>
namespace practice::day17 {

class SequenceUnwrapper {
public:
  [[nodiscard]] std::uint64_t unwrap(std::uint16_t sequence);

private:
  static constexpr int kReorderBound = 64;
  bool initialized_{false};
  std::uint16_t last_sequence_{};
  std::uint64_t last_extended_{};
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day17
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day17 {

std::uint64_t SequenceUnwrapper::unwrap(std::uint16_t sequence) {
  if (!initialized_) {
    initialized_ = true;
    last_sequence_ = sequence;
    last_extended_ = sequence;
    return last_extended_;
  }

  const auto delta = static_cast<std::int16_t>(
      static_cast<std::uint16_t>(sequence - last_sequence_));

  if (delta >= 0) {
    last_extended_ += static_cast<std::uint64_t>(delta);
  } else if (delta >= -kReorderBound) {
    const auto back = static_cast<std::uint64_t>(-delta);
    if (last_extended_ >= back) {
      last_extended_ -= back;
    } else {
      last_extended_ +=
          static_cast<std::uint64_t>(static_cast<std::int32_t>(delta) + 65536);
    }
  } else {
    last_extended_ +=
        static_cast<std::uint64_t>(static_cast<std::int32_t>(delta) + 65536);
  }

  last_sequence_ = sequence;
  return last_extended_;
}

int Solution::run(std::ostream &out) {
  out << "--- day 17 ---\n";
  int ret = 0;

  SequenceUnwrapper rollover;
  ret += is_eq(rollover.unwrap(65535), 65535);
  ret += is_eq(rollover.unwrap(0), 65536);
  ret += is_eq(rollover.unwrap(1), 65537);

  SequenceUnwrapper reorder;
  ret += is_eq(reorder.unwrap(10), 10);
  ret += is_eq(reorder.unwrap(8), 8);
  ret += is_eq(reorder.unwrap(9), 9);

  SequenceUnwrapper far;
  ret += is_eq(far.unwrap(100), 100);
  ret += is_eq(far.unwrap(20), 65556);

  SequenceUnwrapper repeat;
  ret += is_eq(repeat.unwrap(4), 4);
  ret += is_eq(repeat.unwrap(4), 4);

  SequenceUnwrapper floor;
  ret += is_eq(floor.unwrap(0), 0);
  ret += is_eq(floor.unwrap(65535), 65535);
  return ret;
}

} // namespace practice::day17
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day17
./build/basic/cpp_rtc_practice --run 17
```

`run()` must return 0. Rollover advances the extended sequence. A
delta of `-2` inside 64 reorders. A delta of `-80` wraps forward. Do
not use signed 16-bit subtraction, sockets, or sleep.
