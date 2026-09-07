# Day 02 — SIP Header List

Implement public `parse_headers(std::string_view)`. Input: CRLF-delimited `Name: Value` lines; output: an ordered `vector<Header>`. The teaching fixture starts with two `Via` headers and contains nine fields in total. Both `Via` values must remain in input order. Reject a line that lacks `:`; ignore only a trailing CRLF. Focus: structs, vectors, `string_view`. This is not RFC-complete folding support.

## Requirements

- Implement `parse_headers` as a public callable API for the stated CRLF-delimited teaching subset.
- Preserve input order and duplicate field names; reject a line that lacks `:`.

## How to start

1. Complete `starter.cpp` without normalizing duplicate headers away.
2. Put your examples in `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day02`.

## Acceptance test

- **Given** this nine-header block:

  ```
  Via: SIP/2.0/UDP a
  Via: SIP/2.0/TCP b
  From: "Alice" <sip:alice@atlanta.com>;tag=9fxced76sl
  To: Bob <sip:bob@biloxi.com>
  Call-ID: 2xQU9V7rfyb5uhAwH1s83d;1234567890
  CSeq: 314159 INVITE
  Contact: <sip:alice@atlanta.com>
  Content-Type: application/sdp
  Content-Length: 142
  ```

- **When** the block is parsed.
- **Then** the output has nine ordered entries, including both `Via` values; a line without `:` is rejected and stops the parse.
