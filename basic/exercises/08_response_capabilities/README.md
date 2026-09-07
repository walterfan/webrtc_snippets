# Day 08 — Response Capabilities

Implement `status_info` and `check_required_extensions`. Input for status
lookup is a SIP response code; output is a bounded `StatusInfo` (phrase,
`ResponseClass`, and provisional/final flag) or no value. Input for
capability checking is a required option-tag list and a supported list;
output is whether every required tag is accepted plus the unsupported tags
in first-seen order. Focus: `constexpr` lookup, `array`, enum classification,
and deterministic set-like comparison. Never implement Digest, generate
responses, or route through a proxy.

## Requirements

### Bounded status table

Keep a teaching-only constexpr table. Exact codes in scope:

| Code | Teaching phrase |
| --- | --- |
| 100 | Trying |
| 180 | Ringing |
| 183 | Session Progress |
| 200 | OK |
| 202 | Accepted |
| 301 | Moved Permanently |
| 302 | Moved Temporarily |
| 401 | Unauthorized |
| 404 | Not Found |
| 407 | Proxy Authentication Required |
| 420 | Bad Extension |
| 422 | Session Interval Too Small |
| 423 | Interval Too Brief |
| 480 | Temporarily Unavailable |
| 481 | Call/Transaction Does Not Exist |
| 486 | Busy Here |
| 487 | Request Terminated |
| 488 | Not Acceptable Here |
| 491 | Request Pending |
| 500 | Server Internal Error |
| 503 | Service Unavailable |
| 603 | Decline |
| 604 | Does Not Exist Anywhere |

- Classify by hundred-block into `ResponseClass`: `1xx` provisional, `2xx`
  success, `3xx` redirection, `4xx` client_error, `5xx` server_error, `6xx`
  global_failure.
- Set `final` to `false` only for provisional (`1xx`) codes; all other table
  entries are final.
- Distinguish UAS challenge `401 Unauthorized` from proxy challenge
  `407 Proxy Authentication Required` as separate table rows — do not collapse
  them.
- Return `std::nullopt` for any code not in the table (for example `999`). An
  unknown code is simply outside the constexpr set; do not invent phrases.
- A `constexpr status_info` definition must be visible at its call site.
  Define the function body in `starter.hpp` (a `constexpr` function is
  implicitly inline). This skeleton is declaration-only and does not define
  it. A definition that lives only in `starter.cpp` cannot be used as a
  constant expression from another translation unit.

### Option tags (`Supported` / `Require` / `Proxy-Require` / `Unsupported`)

- Treat `required` as the tags a peer insists on (teaching stand-in for
  `Require` / `Proxy-Require` values) and `supported` as the tags this side
  advertises (`Supported`).
- Accept the request only when every required tag appears in `supported`
  (compare case-insensitively after trimming; do not mutate the caller's
  vectors).
- When a required tag is missing, collect it into `CapabilityResult::unsupported`
  in first-seen order, without duplicate diagnostics for the same tag.
- `Supported` alone does not satisfy a missing `Require`; listing a tag as
  supported does not mean it was required.

## Out of scope

Digest challenge algorithms, response-message generation, and proxy routing
are not part of this exercise. `420 Bad Extension` is a status-table entry
only; you do not build the `Unsupported` header wire format here beyond the
`CapabilityResult::unsupported` vector.

## How to start

1. Read the public contract in `starter.hpp` (`ResponseClass`, `StatusInfo`,
   `status_info`, `CapabilityResult`, `check_required_extensions`).
2. Define `constexpr status_info` in `starter.hpp` so the definition is
   visible at every call site. Implement `check_required_extensions` in
   `starter.cpp`. Keep `Solution::run` as a render helper until the lookups
   are ready. The learner header stays declaration-only until you add that
   body.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day08` after wiring a real check.

## Acceptance fixtures

### Happy path — provisional versus final

- **Given** codes `180` and `200`
- **When** `status_info` runs
- **Then** `180` is provisional (`final == false`, category `provisional`,
  phrase `Ringing`) and `200` is final success (`final == true`, category
  `success`, phrase `OK`)

### Happy path — UAS versus proxy challenge

- **Given** codes `401` and `407`
- **When** `status_info` runs
- **Then** both are final `client_error` entries with phrases `Unauthorized`
  and `Proxy Authentication Required` respectively

### Happy path — session / registration interval errors

- **Given** codes `422` and `423`
- **When** `status_info` runs
- **Then** both resolve as final `client_error` with phrases
  `Session Interval Too Small` and `Interval Too Brief`

### Happy path — call / dialog / media errors

- **Given** codes `481`, `487`, `488`, and `491`
- **When** `status_info` runs
- **Then** each resolves as final `client_error` with the teaching phrases
  from the table above

### Happy path — global failure and service unavailable

- **Given** codes `503`, `603`, and `604`
- **When** `status_info` runs
- **Then** `503` is final `server_error`; `603` and `604` are final
  `global_failure`

### Reject — unknown status

- **Given** code `999`
- **When** `status_info` runs
- **Then** the result is empty (`std::nullopt`)

### Happy path — all required tags supported

- **Given** `required = {"100rel", "timer"}` and
  `supported = {"100rel", "timer", "path"}`
- **When** `check_required_extensions` runs
- **Then** `accepted == true` and `unsupported` is empty

### Reject — unsupported required tag

- **Given** `required = {"100rel", "timer"}` and `supported = {"100rel"}`
- **When** `check_required_extensions` runs
- **Then** `accepted == false` and `unsupported` is `{"timer"}` in that order

### Reject — duplicate required tags

- **Given** `required = {"timer", "100rel", "timer"}` and
  `supported = {"100rel"}`
- **When** `check_required_extensions` runs
- **Then** `accepted == false` and `unsupported` lists `timer` once (first
  seen), then does not repeat it
