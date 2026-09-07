# Day 12 — Session Refresh

Implement `negotiate_session_timer`, `refresh_due`, `refresh_method_allowed`,
and `is_glare_retry` for a bounded RFC 4028 Session Timer plus RFC 3311
UPDATE profile. Input is a session-timer offer (interval and optional
refresher), a local `Min-SE`, a fake elapsed duration, a refresh method with
early/confirmed dialog context, and a response status code. Focus: `chrono`
durations, `std::variant` success/failure, optional refresher roles, and
deterministic fake-time decisions. Never sleep, read wall clocks, or send a
refresh request.

## Requirements

### Protocol roles (RFC 4028 / RFC 3311 teaching subset)

- `Session-Expires` is a **signaling** session lifetime. It is not RTP
  keepalive and not a NAT binding refresh.
- `Min-SE` is the minimum acceptable session interval. An offer below that
  floor is rejected in this lab as typed `too_small` data (analogous to a
  `422 Session Interval Too Small` response), carrying the required minimum.
- Zero or negative offered intervals are `invalid_interval` (not a 422
  floor problem).
- When the offer omits a refresher role, this profile defaults to
  `Refresher::uac`.
- The refresher becomes due at **half** the negotiated interval, using the
  caller-supplied fake `elapsed` duration — no wall clock.
- In this bounded profile, `RefreshMethod::update` is allowed on an early
  dialog; `RefreshMethod::reinvite` is allowed only on a confirmed dialog
  (`early_dialog == false`).
- Status `491` is classified as a glare retry (`Request Pending`). Other
  codes are not.

### `negotiate_session_timer`

- Reject non-positive `offer.interval` as
  `SessionTimerFailure{invalid_interval, required_min=0s}` (no floor
  applies; this is not a 422-shaped case).
- When the interval is positive but `offer.interval < min_se`, return
  `SessionTimerFailure{too_small, required_min=min_se}` (422-shaped
  teaching result).
- On success, return `SessionTimer` with the offered interval and an
  explicit refresher: use `offer.refresher` when present, otherwise
  `Refresher::uac`.

### `refresh_due`

- Return `true` when `elapsed >= timer.interval / 2` (integer
  `std::chrono::seconds` arithmetic).
- Return `false` when elapsed is still strictly before the half-interval.

### `refresh_method_allowed`

- `update`: allowed for both early and confirmed dialogs in this profile.
- `reinvite`: allowed only when `early_dialog == false`.

### `is_glare_retry`

- Return `true` only for response code `491`.
- Return `false` for every other code (including other `4xx` values).

### Out of scope

Sleeping, `std::chrono::system_clock` / steady-clock reads, socket I/O,
building UPDATE or re-INVITE messages, and RTP/NAT keepalive logic are
**not** part of this exercise.

## How to start

1. Read the public contract in `starter.hpp` (`SessionTimerOffer`,
   `SessionTimer`, `SessionTimerFailure`, and the four free functions).
2. Implement the targets in `starter.cpp`; keep `Solution::run` as a render
   helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day12` after wiring a real check.

## Acceptance fixtures

### Happy path — omitted refresher defaults to UAC

- **Given** offer `{interval=1800s, refresher=nullopt}` and `min_se=90s`
- **When** `negotiate_session_timer` runs
- **Then** success with `interval=1800s` and `refresher=uac`

### Happy path — explicit UAS refresher is preserved

- **Given** offer `{interval=1800s, refresher=uas}` and `min_se=90s`
- **When** `negotiate_session_timer` runs
- **Then** success with `refresher=uas`

### Reject — too-small interval (422-shaped)

- **Given** offer `{interval=30s, refresher=nullopt}` and `min_se=90s`
- **When** `negotiate_session_timer` runs
- **Then** `SessionTimerFailure{too_small, required_min=90s}`

### Reject — zero interval is invalid

- **Given** offer `{interval=0s, ...}` and any positive `min_se`
- **When** `negotiate_session_timer` runs
- **Then** failure with `kind=invalid_interval`

### Reject — negative interval is invalid

- **Given** offer `{interval=-1s, ...}` and any positive `min_se`
- **When** `negotiate_session_timer` runs
- **Then** failure with `kind=invalid_interval`

### Happy path — refresh due at half interval

- **Given** timer `{interval=1800s, refresher=uac}` and `elapsed=900s`
- **When** `refresh_due` runs
- **Then** the result is `true`

### Happy path — not due before half interval

- **Given** timer `{interval=1800s, refresher=uac}` and `elapsed=899s`
- **When** `refresh_due` runs
- **Then** the result is `false`

### Happy path — UPDATE allowed in early dialog

- **Given** `RefreshMethod::update` and `early_dialog=true`
- **When** `refresh_method_allowed` runs
- **Then** the result is `true`

### Reject — re-INVITE forbidden in early dialog

- **Given** `RefreshMethod::reinvite` and `early_dialog=true`
- **When** `refresh_method_allowed` runs
- **Then** the result is `false`

### Happy path — re-INVITE allowed in confirmed dialog

- **Given** `RefreshMethod::reinvite` and `early_dialog=false`
- **When** `refresh_method_allowed` runs
- **Then** the result is `true`

### Happy path — 491 is glare retry

- **Given** response code `491`
- **When** `is_glare_retry` runs
- **Then** the result is `true`

### Reject — other codes are not glare

- **Given** response code `500` (or `200`, `422`, …)
- **When** `is_glare_retry` runs
- **Then** the result is `false`
