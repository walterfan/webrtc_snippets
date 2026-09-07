# Day 27 — SRTP Replay Window

Implement `ReplayWindow::accept`. Example: accept 10, reject replayed 10, and accept 9 when within the stated window. Test an expired sequence and a rollover boundary. Use unsigned arithmetic and bounded bit operations. This is a replay policy exercise, not SRTP cryptography.

## Requirements

- Track the documented bounded replay window with unsigned sequence arithmetic.
- Reject duplicates and expired packets; define rollover behavior explicitly.

## How to start

1. Implement window state and bit operations in `starter.cpp`.
2. Add first, replay, reorder, expiry, and rollover cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day27`.

## Acceptance test

- **Given** sequences 10, replayed 10, and reordered 9 within the window.
- **When** `accept` runs.
- **Then** 10 and 9 are accepted once, the replay is rejected, and expired/rollover cases follow the documented rule.
