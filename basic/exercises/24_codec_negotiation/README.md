# Day 24 — Codec Negotiation

Implement `negotiate`. Select the first preferred compatible codec from fixed offer/answer vectors; example: matching `opus/48000`. Return no value when none match and reject duplicate payload types in one side. Define a small fmtp compatibility rule. Focus: ranges, set algorithms, projections.

## Requirements

- Choose the first preferred compatible codec under the documented name/rate/fmtp rule.
- Return no value for no match and reject duplicate payload types in either list.

## How to start

1. Implement matching with ranges or set algorithms in `starter.cpp`.
2. Add preferred match, no-match, duplicate, and fmtp cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day24`.

## Acceptance test

- **Given** offer/answer lists with matching `opus/48000`, a no-match pair, and duplicate payload data.
- **When** codec negotiation runs.
- **Then** it selects Opus, returns no value for no match, and rejects duplicate payload types.
