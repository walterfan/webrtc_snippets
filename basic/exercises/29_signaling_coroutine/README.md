# Day 29 — Coroutine Signaling Flow

Implement `SignalTask` and `negotiate_offer_answer`. A static offer event must occur before its answer; an injected local failure must be observable. Define ownership of any coroutine handle and avoid external event loops. Focus: C++20 coroutines, awaiters, exception propagation.

## Requirements

- Keep coroutine-handle ownership explicit and local.
- Emit offer before answer and propagate injected local failure without an external event loop.

## How to start

1. Implement `SignalTask` and the deterministic sequence in `starter.cpp`.
2. Add event-order and exception checks to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day29`.

## Acceptance test

- **Given** a static offer and an injected local failure path.
- **When** the signaling coroutine runs.
- **Then** answer follows offer on success and the failure is observable through the documented error path.
