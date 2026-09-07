# Day 06 — Via Routing

Implement `parse_via_hop`. Input is **one Via header value** (not the
`Via:` name), beginning with `SIP/2.0/<transport>`. Parse the transport token,
the sent-by host (and optional port), and semicolon-delimited parameters.
Focus: `string_view` token boundaries, case-insensitive parameter names,
`optional`, and owned return values.

## Requirements

- Require the prefix `SIP/2.0/` followed by a known transport token mapped to
  `Transport`: `UDP`, `TCP`, `TLS`, `WS`, or `WSS` (compare
  case-insensitively).
- Require a non-empty sent-by token after the transport and exactly one
  non-empty `branch` parameter.
- Parameter names are case-insensitive (`Branch`, `RECEIVED`, `RPort`).
- Optional `received` copies the observed address string into
  `ViaHop::received` without replacing `sent_by`.
- `rport` semantics (RFC 3581 teaching subset):
  - absent → `rport_requested == false`, `rport` empty;
  - present with no value (`rport` or `rport=`) → `rport_requested == true`,
    `rport` empty;
  - present with a decimal value → `rport_requested == true` and
    `rport` holds that port when it is in the teaching range
    **0 through 65535** inclusive.
- Unknown parameters are ignored only when their `name` / `name=value`
  syntax is valid; malformed parameter syntax still fails the parse.
- Reject missing or duplicate `branch`, empty sent-by, unknown transport,
  non-decimal or out-of-range numeric `rport`, and other malformed input by
  returning `std::nullopt`.

## Out of scope

DNS resolution of sent-by, NAT traversal, TLS certificate validation, and
sending responses are **not** part of this exercise. Transport names are
data only.

## How to start

1. Read the public contract in `starter.hpp` (`ViaHop`, `parse_via_hop`).
2. Implement `parse_via_hop` in `starter.cpp`; keep `Solution::run` as a
   render helper until the parser is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day06` after wiring a real check.

## Acceptance fixtures

### Happy path — UDP with received and numeric rport

- **Given**
  `SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bK-1;received=192.0.2.1;rport=5070`
- **When** `parse_via_hop` runs
- **Then** transport is `udp`, `sent_by` is `pc33.atlanta.com`, `branch` is
  `z9hG4bK-1`, `received` is `192.0.2.1`, `rport` is `5070`, and
  `rport_requested` is `true`

### Happy path — TLS with empty rport request

- **Given** `SIP/2.0/TLS proxy.example.com:5061;branch=z9hG4bK776asdhds;rport`
- **When** `parse_via_hop` runs
- **Then** transport is `tls`, `sent_by` is `proxy.example.com:5061`,
  `branch` is `z9hG4bK776asdhds`, `received` is empty, `rport` is empty, and
  `rport_requested` is `true`

### Happy path — mixed-case parameter names

- **Given**
  `SIP/2.0/UDP host.example;Branch=z9hG4bKabc;Received=198.51.100.9;RPort=5080`
- **When** `parse_via_hop` runs
- **Then** the same fields populate as for lowercase names (`branch`,
  `received`, numeric `rport`, `rport_requested == true`)

### Reject — missing branch

- **Given** `SIP/2.0/UDP host.example;received=192.0.2.1`
- **When** `parse_via_hop` runs
- **Then** the result is empty

### Reject — duplicate branch

- **Given** `SIP/2.0/UDP host.example;branch=z9hG4bK-1;branch=z9hG4bK-2`
- **When** `parse_via_hop` runs
- **Then** the result is empty

### Reject — out-of-range port

- **Given** `SIP/2.0/UDP host.example;branch=z9hG4bK-1;rport=70000`
- **When** `parse_via_hop` runs
- **Then** the result is empty
