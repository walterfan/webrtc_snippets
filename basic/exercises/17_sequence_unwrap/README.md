# Day 17 — RTP Sequence Unwrap

Implement `SequenceUnwrapper::unwrap`. Input: 16-bit RTP sequence values;
output: a monotonically extended `std::uint64_t` under the **64-entry
bounded half-range** policy below. Example: `65535`, then `0`, crosses one
rollover. Include a bounded reordered input and avoid signed overflow.
Focus: fixed-width integers and value semantics.

## Requirements

### Storage

The unwrapper keeps three pieces of private state:

- `initialized_` — false until the first `unwrap`
- `last_sequence_` — last 16-bit sequence passed to `unwrap`
- `last_extended_` — last returned 64-bit extended sequence

`kReorderBound` is **64**. Do not add public fields.

### 64-entry bounded half-range policy

Sequence space is 16-bit, modulus `65536`. Both operands are
`std::uint16_t`, but usual arithmetic conversions promote them to
`int` before subtraction, so `0 - 65535` is the signed `int` value
`-65535`, not a 16-bit wrap. Force modular 16-bit wrap first, then
read the result as a half-range signed step:

```
delta = static_cast<std::int16_t>(
    static_cast<std::uint16_t>(sequence - last_sequence_));
```

That `uint16_t` cast is the modular wrap (`-65535` → `1`). The
following `int16_t` cast maps the wrapped difference into
`[-32768, 32767]`. Example: `65535` then `0` gives `delta == 1`. Then:

| `delta` | Meaning | Extended update |
| --- | --- | --- |
| `0` | same sequence again | return `last_extended_` unchanged |
| `> 0` | forward, including `65535 → 0` (`delta == 1`) | `last_extended_ += delta` |
| `[-64, -1]` and `last_extended_ >= -delta` | reorder within the 64-entry window | subtract `-delta` from `last_extended_` |
| `[-64, -1]` and `last_extended_ < -delta` | in-window reorder would go below 0 (**floor / underflow**) | treat as a **forward wrap**: add `delta + 65536` |
| `< -64` | too far backward to be a 64-window reorder; treat as a **forward wrap** | `last_extended_ += (delta + 65536)` |

After a successful step, store `last_sequence_ = sequence` and
`last_extended_` as the return value. Apply the signed step to
`std::uint64_t` without converting a negative `delta` through an
unsigned widening that would wrap to a huge increment: add when
non-negative; subtract `-delta` when the in-window reorder stays
non-negative. The floor row is the exception: `0` then `65535` has
`delta == -1` but must become `65535`, not a wrapped `uint64` underflow.

### First call

The first `unwrap(s)` sets `initialized_ = true`,
`last_sequence_ = s`, `last_extended_ = s`, and returns `s`.

### Out of scope

RTCP, jitter buffering, sockets, and wall-clock waits are **not** part
of this exercise. Day 19 owns late/duplicate packet storage.

## How to start

1. Read the private initialization / last-sequence / last-extended
   members in `starter.hpp`.
2. Implement `unwrap` in `starter.cpp`; keep `Solution::run` as a render
   helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall.
4. Run `ctest --test-dir build/basic -R Day17` after wiring a real check.

## Acceptance fixtures

### Happy path — rollover

- **Given** `65535` then `0`
- **When** each is unwrapped
- **Then** results are `65535` then `65536`

### Happy path — in-window reorder

- **Given** `10` then `8` then `9`
- **When** each is unwrapped
- **Then** results are `10`, `8`, `9` (`delta` of `-2` then `+1`)

### Bounded wrap, not a far reorder

- **Given** `100` then `20`
- **When** each is unwrapped
- **Then** `20` is **not** treated as `100 - 80` (80 > 64); it is a
  forward wrap and the extended value is `100 + 65456` = `65556`

### Repeat

- **Given** the same sequence twice in a row
- **When** `unwrap` runs
- **Then** the second call returns the same extended value

### Floor / underflow — in-window reorder below 0

- **Given** `0` then `65535`
- **When** each is unwrapped
- **Then** results are `0` then `65535` (`delta == -1` would drop
  below 0, so it is a forward wrap, not `uint64` wrap-around)
