# Day 01 — SIP Request-Line

Implement `parse_request_line`. Input: one ASCII request line; output: method, URI, and SIP version views. Example: `INVITE sip:bob@example.com SIP/2.0`. Reject blank input, missing fields, extra fields, an empty Request-URI, and non-`SIP/2.0` versions. Focus: C++20 `string_view`, `std::optional`, value types. Teaching subset only; not a complete SIP parser.

## Requirements

- Implement `parse_request_line` without returning views into temporary storage.
- Accept exactly three space-separated fields and require `SIP/2.0` as the version.
- Reject blank input, a missing method or URI, extra fields, an empty URI, and any version other than `SIP/2.0`.

## How to start

1. Edit `starter.cpp` and implement the declaration.
2. Add the focused assertions to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day01`.

## Acceptance test

- **Given** `INVITE sip:bob@example.com SIP/2.0`.
- **When** it is parsed.
- **Then** method, URI, and version equal the three fields.
- **And** these inputs are rejected:
  - blank / empty input
  - missing fields (`INVITE SIP/2.0`)
  - empty URI (`INVITE  SIP/2.0`)
  - extra fields (`INVITE sip:bob@example.com SIP/2.0 extra`)
  - non-`SIP/2.0` version (`INVITE sip:bob@example.com SIP/1.0`)
