# Day 28 — DataChannel Fragment Reassembly

Implement `Reassembler`. Two fragments for message ID 7 form one ordered byte vector after the final fragment. Reject duplicate fragments, bound incomplete messages, and let `abandon` release state. Focus: maps, variants, ownership. This is an in-memory teaching model, not SCTP.

## Requirements

- Own fragment bytes and bound the amount of incomplete-message state.
- Deliver a message once, reject duplicates, and release state through `abandon`.

## How to start

1. Implement fragment bookkeeping in `starter.cpp`.
2. Add ordered, duplicate, missing, and abandoned cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day28`.

## Acceptance test

- **Given** two fragments for message ID 7, a duplicate, and an abandoned message.
- **When** fragments are pushed.
- **Then** ID 7 is delivered once in order, the duplicate is rejected, and abandoned state is released.
