# Day 14 — Protocol Errors

Implement `parse_content_length` and `diagnose` for a typed SIP
parse/validation boundary. Input is a Content-Length header **value**
(not the header name) or an `ErrorKind` plus owned detail text. Focus:
`std::variant`, exception containment, numeric validation, and preserving
error context. Never open sockets, throw across the public API, or add
executable PAI / UUI / SIPREC APIs.

## Requirements

### `parse_content_length`

- Trim surrounding spaces and tabs (`' '` / `'\t'`) only; do not strip
  other whitespace.
- After trim, accept a decimal digit string representing an integer from
  **0 through 65535** inclusive.
- Leading zeroes are allowed (`"0042"` → `42`, `"0"` → `0`).
- On success, return the value as `unsigned short` in
  `ContentLengthResult`.
- Reject and return `ProtocolError` (do **not** throw). Kind rules are
  normative:

  | Failure | `ErrorKind` | `response_code` |
  | ------- | ----------- | --------------- |
  | Empty (including whitespace-only after trim) | `malformed` | 400 |
  | Leading sign (`+` / `-`) | `malformed` | 400 |
  | Non-digit character in the token | `malformed` | 400 |
  | Partial / incomplete consumption (e.g. `"42x"`) | `malformed` | 400 |
  | Numeric value outside 0…65535 (over-limit or overflowing digit string) | `out_of_range` | 400 |

- Do **not** encode the malformed-versus-range split only in `detail`;
  use the distinct enum enumerators above.
- The public contract must **not** leak a parsing exception
  (`std::stoul` / `from_chars` failures must be caught or avoided).

### `diagnose`

Map each `ErrorKind` to its normal SIP response code and return a
`ProtocolError` that owns `detail`:

| `ErrorKind`              | Response | Meaning (teaching)                          |
| ------------------------ | -------- | ------------------------------------------- |
| `malformed`              | 400      | Bad Request (syntax / incomplete input)     |
| `out_of_range`           | 400      | Bad Request (numeric outside teaching range)|
| `too_many_hops`          | 483      | Max-Forwards exhausted                      |
| `unsupported_extension`  | 420      | Required option tag not supported           |
| `auth_challenge`         | 401      | Challenge from a UA / registrar             |
| `interval_too_small`     | 422      | Session-Timer interval too small            |
| `interval_too_brief`     | 423      | Registration / expires too brief            |
| `dialog_not_found`       | 481      | Call/Transaction Does Not Exist             |
| `media_not_acceptable`   | 488      | Not Acceptable Here (offer/answer)          |
| `request_pending`        | 491      | Request Pending (glare)                     |
| `server_unavailable`     | 503      | Service Unavailable                         |

- Document **407** as the **proxy** equivalent of **401**: both are
  authentication challenges; `diagnose(auth_challenge, …)` returns
  **401**, while a proxy hop would challenge with 407. Same concept,
  different challenger — do not invent a second `ErrorKind`.
- `diagnose(malformed, …)` and `diagnose(out_of_range, …)` both return
  **400**; the kind field still distinguishes syntax from range.
- Copy `detail` into the returned object; do not alias the caller's
  temporary.

### Out of scope

Cryptographic verification of Identity, rewriting History-Info at the
wire, UUI authorization, SIPREC media/metadata recording, and building
full SIP response messages are **not** part of this exercise.

## Call-center trust-boundary context (reading only)

Use these as diagnostic reading notes — **not** extra APIs to implement:

- **P-Asserted-Identity (PAI):** trusted only inside a configured trust
  domain. Outside that domain, treat PAI as untrusted assertion text.
- **Reason / History-Info:** may be rewritten, truncated, or incomplete
  across hops; useful for diagnostics, not as sole proof of cause.
- **User-to-User (UUI):** bounded opaque context carried with the call;
  do not interpret it as authenticated customer identity.
- **Identity (STIR/SHAKEN-style):** a verifiable assertion about the
  calling number — not the same as authenticating the end customer to
  your application.
- **SIPREC:** the recording session is a **separate** dialog/leg from
  the customer media dialog; metadata about recording must not be
  confused with customer signaling state.

## How to start

1. Read the public contract in `starter.hpp` (`ErrorKind`,
   `ProtocolError`, `ContentLengthResult`, `parse_content_length`,
   `diagnose`).
2. Implement both functions in `starter.cpp`; keep `Solution::run` as a
   render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a
   finished solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day14` after wiring a real
   check.

## Acceptance fixtures

### Happy path — plain decimal

- **Given** `"42"`
- **When** `parse_content_length` runs
- **Then** the result holds `unsigned short{42}`

### Happy path — leading zeroes and surrounding trim

- **Given** `"  0042\t"`
- **When** `parse_content_length` runs
- **Then** the result holds `42`

### Happy path — zero and upper bound

- **Given** `"0"` and `"65535"`
- **When** each is parsed
- **Then** both succeed with `0` and `65535`

### Reject — empty / whitespace-only

- **Given** `""` or `"   "`
- **When** `parse_content_length` runs
- **Then** the result is a `ProtocolError` with `kind == malformed`
  and `response_code == 400`

### Reject — signed or non-digit

- **Given** `"-1"`, `"+3"`, or `"oops"`
- **When** `parse_content_length` runs
- **Then** each yields `ProtocolError` with `kind == malformed` and
  `response_code == 400`

### Reject — incomplete consumption

- **Given** `"42x"`
- **When** `parse_content_length` runs
- **Then** the result is `ProtocolError` with `kind == malformed` and
  `response_code == 400` (digits must consume the entire trimmed token)

### Reject — over-limit / overflowing

- **Given** `"65536"` and a long all-digit string such as `"999999"`
- **When** `parse_content_length` runs
- **Then** each yields `ProtocolError` with `kind == out_of_range` and
  `response_code == 400` (still no exception leaves the API)

### Happy path — diagnose mapping

- **Given** each `ErrorKind` in the table above (including `malformed`
  and `out_of_range`)
- **When** `diagnose(kind, "sample")` runs
- **Then** `response_code` matches the table and `detail` is `"sample"`

### Document — 401 vs 407

- **Given** `ErrorKind::auth_challenge`
- **When** `diagnose` runs
- **Then** `response_code == 401`, with README/tips noting 407 as the
  proxy-challenge equivalent (not a separate enum enumerator)
