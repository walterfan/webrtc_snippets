# Day 26 — DTLS Fingerprint Syntax

Implement `validate_fingerprint`. Accept only the documented algorithm token (for example `sha-256`) and colon-separated uppercase/lowercase hex pairs of the expected length. Reject malformed text and unknown algorithms. This validates syntax only; it does not hash, compare certificates, or authenticate peers. Focus: ranges and validated value types.

## Requirements

- Validate only an allowlisted algorithm and bounded colon-separated hex syntax.
- Reject malformed input and never claim certificate or peer authentication.

## How to start

1. Implement textual validation in `starter.cpp`.
2. Add valid, malformed, wrong-length, and unknown-algorithm cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day26`.

## Acceptance test

- **Given** a valid `sha-256` fingerprint, malformed hex, and an unknown algorithm.
- **When** validation runs.
- **Then** only the valid textual form is accepted; no cryptographic verification is attempted.
