# Day 21 — Parallel ICE Checks

Implement `check_pairs` using only fake local outcomes. Example pairs produce success, timeout, and failed results in input order. Propagate task exceptions into a defined failure result and bound any wait by the supplied timeout. Focus: `async`, `future`, timeouts. No sockets or STUN/TURN traffic.

## Requirements

- Model fixed local outcomes only; do not create sockets or contact STUN/TURN.
- Preserve input order, bound waits by the supplied timeout, and map exceptions to failure.

## How to start

1. Implement fake asynchronous checks in `starter.cpp`.
2. Add success, timeout, and exception cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day21`.

## Acceptance test

- **Given** three fake candidate pairs producing success, timeout, and exception.
- **When** `check_pairs` runs with a finite timeout.
- **Then** it returns success, timeout, and failure in input order without network traffic.
