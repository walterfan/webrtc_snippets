# Day 10 — Reliable Provisional Responses

Implement `parse_rack`, `requires_prack`, and `matches_prack`. Input is a
single `RAck` header value (not the `RAck:` name), a provisional status code
plus a `100rel` reliability flag, and the pending RSeq, INVITE CSeq, and
expected method that must agree with a parsed `RAck`. Focus: bounded text
parsing, checked numeric conversion, and matching independent protocol
identifiers. Never schedule retransmits, parse SDP, or transport Early Media.

## Requirements

### Protocol roles (RFC 3262 teaching subset)

- `100rel` marks that a non-100 provisional response is sent reliably and
  expects a PRACK acknowledgment.
- A reliable provisional carries an `RSeq` sequence number. The UAC answers
  with a PRACK whose `RAck` echoes that `RSeq`, the related INVITE `CSeq`
  number, and the INVITE method token.
- `183 Session Progress` is the usual Early Media / reliable-progress case
  in this lab; treat it like any other reliable non-100 provisional when
  deciding PRACK.
- **`100 Trying` is excluded** from reliable provisional handling. It never
  requires PRACK under these rules, even if a `100rel` flag is present.

### `parse_rack`

- Grammar is exactly three fields: `rseq` SP `invite_cseq` SP `method`
  (horizontal whitespace around tokens is allowed; consume the full input).
- `rseq` and `invite_cseq` are unsigned decimal values; reject zero,
  non-decimal residue, and values that overflow `unsigned`.
- `method` is a non-empty token (teaching fixture: `INVITE`). Extra tokens,
  duplicate numeric fields, or a truncated value yield `std::nullopt`.
- On success, return an owned `RAck` (`std::string` method); do not alias the
  caller's `string_view`.

### `requires_prack`

- Return `true` only for a **non-100 provisional** status (`101`–`199`) when
  `has_100rel` is true.
- Return `false` for `100`, for final responses (`>= 200`), and whenever
  `has_100rel` is false — including an ordinary `180` without reliability.

### `matches_prack`

- Compare the parsed `RAck` against separately supplied pending identifiers:
  `rseq`, `invite_cseq`, and `expected_method` (`std::string_view`).
- Return `true` only when `rack.rseq`, `rack.invite_cseq`, and `rack.method`
  all equal those three arguments.
- A mismatch on any one field is a failure.

### Out of scope

Retransmission timers, SDP bodies on `183`, RTP Early Media, and full SIP
message construction are **not** part of this exercise.

## How to start

1. Read the public contract in `starter.hpp` (`RAck`, `parse_rack`,
   `requires_prack`, `matches_prack`).
2. Implement the three functions in `starter.cpp`; keep `Solution::run` as a
   render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day10` after wiring a real check.

## Acceptance fixtures

### Happy path — reliable 183 requires PRACK

- **Given** status `183` and `has_100rel == true`
- **When** `requires_prack` runs
- **Then** the result is `true`

### Happy path — ordinary 180 without 100rel

- **Given** status `180` and `has_100rel == false`
- **When** `requires_prack` runs
- **Then** the result is `false`

### Happy path — 100 Trying never requires PRACK

- **Given** status `100` and `has_100rel == true`
- **When** `requires_prack` runs
- **Then** the result is `false`

### Happy path — matching RAck

- **Given** pending `rseq == 1`, INVITE `cseq == 1`,
  `expected_method == "INVITE"`, and `RAck` value `1 1 INVITE`
- **When** `parse_rack` then `matches_prack` run
- **Then** parse succeeds and matching returns `true`

### Reject — mismatched RSeq

- **Given** pending `rseq == 2` but `RAck` parses to `rseq == 1` (same INVITE
  cseq and `expected_method`)
- **When** `matches_prack` runs
- **Then** the result is `false`

### Reject — mismatched INVITE CSeq

- **Given** pending INVITE `cseq == 7` but `RAck` carries a different
  `invite_cseq` (same `rseq` and `expected_method`)
- **When** `matches_prack` runs
- **Then** the result is `false`

### Reject — mismatched method

- **Given** `expected_method == "INVITE"` but `RAck` method is `ACK` (or
  another non-matching token)
- **When** `matches_prack` runs
- **Then** the result is `false`

### Reject — extra tokens

- **Given** `RAck` value `1 1 INVITE EXTRA`
- **When** `parse_rack` runs
- **Then** the result is empty

### Reject — duplicate / truncated fields

- **Given** `RAck` values such as `1 1` (missing method) or `1 1 INVITE 1`
  (extra field after method)
- **When** `parse_rack` runs
- **Then** the result is empty

### Reject — zero or overflowing numbers

- **Given** `RAck` values with `0` in either numeric field, or a digit string
  that does not fit in `unsigned`
- **When** `parse_rack` runs
- **Then** the result is empty
