# Day 18 — RTCP Receiver Statistics

Implement `make_report`. Example: expected 10, received 8 gives `0.2`
loss. Preserve supplied jitter and specify when LSR/DLSR yields no RTT
estimate. Handle zero expected packets without division by zero. Focus:
numeric algorithms, accumulation, floating-point boundaries.

## Requirements

This is a **value API**. `make_report` must not store state between
calls. Use only the fields of `ReceiverInputs`.

### Loss fraction

- If `expected == 0`, set `loss_fraction = 0.0`. Do **not** divide.
- If `received >= expected` (and `expected > 0`), set
  `loss_fraction = 0.0` (duplicates or surplus; no negative loss).
- Otherwise
  `loss_fraction = (expected - received) / double(expected)`.
  The teaching vector `expected=10`, `received=8` is exactly `0.2`.

### Jitter

Copy `input.jitter` into `ReceiverReport::jitter` unchanged. Do not
recompute RFC 3550 interarrival jitter and do not wait on a clock.

### RTT / LSR / DLSR

RFC 3550’s sender-side RTT sample is `A - LSR - DLSR`, where `A` is the
reception NTP timestamp of the report that echoed `LSR`. This teaching
API has **no** `A`:

- If `lsr == 0` or `dlsr == 0`, there is no completed SR/RR RTT sample.
- Even when both are non-zero, this function still cannot form `A`
  without a wall-clock or a caller-supplied arrival time.

Therefore `round_trip` is always `std::nullopt`. Do not invent a delay
from `dlsr` alone and do not call `now()`.

### Out of scope

Sending compound RTCP, NTP conversion, sockets, and wall-clock sampling
are **not** part of this exercise.

Day 22 (next task) will snapshot atomic packet/byte counters. That
snapshot is **per-counter atomic**, not pair-consistent, unless that
task adds a lock or seqlock. Do not implement Day 22 here.

## How to start

1. Implement `make_report` in `starter.cpp` from the rules above.
2. Cover loss, zero expected packets, surplus received, and absent RTT
   in `tests/acceptance_test.cpp` when you wire a real check.
3. Run `ctest --test-dir build/basic -R Day18`.

## Acceptance fixtures

### Happy path — loss 0.2

- **Given** `expected=10`, `received=8`, `jitter=4.5`
- **When** `make_report` runs
- **Then** `loss_fraction == 0.2`, `jitter == 4.5`,
  `round_trip == nullopt`

### Zero expected

- **Given** `expected=0`, `received=0` (or any received)
- **When** `make_report` runs
- **Then** `loss_fraction == 0.0` and no division occurs

### Surplus received

- **Given** `expected=5`, `received=7`
- **When** `make_report` runs
- **Then** `loss_fraction == 0.0`

### No RTT

- **Given** any LSR/DLSR pair, including both zero and both non-zero
- **When** `make_report` runs
- **Then** `round_trip` is empty
