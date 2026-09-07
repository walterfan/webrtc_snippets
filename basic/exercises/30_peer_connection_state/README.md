# Day 30 — PeerConnection State Orchestration

Implement `PeerConnectionState`. A conforming observer sees `new -> connecting -> connected`; invalid transitions are rejected. The observer concept requires `on_state(ConnectionState) -> void`. Keep the model local and explicit about lifecycle cleanup. Focus: concepts, generic observer, state composition, RAII.

## Requirements

- Constrain observers to `on_state(ConnectionState) -> void`.
- Notify valid transitions, reject invalid transitions, and clean up local state deterministically.

## How to start

1. Implement the state transition policy in `starter.cpp`.
2. Add a conforming observer and invalid-observer/transition cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day30`.

## Acceptance test

- **Given** a conforming observer and `new -> connecting -> connected` transitions.
- **When** transitions are applied.
- **Then** the observer receives each valid state, while invalid transitions or nonconforming observers are rejected.
