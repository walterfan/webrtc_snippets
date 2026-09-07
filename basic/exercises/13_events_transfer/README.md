# Day 13 — Events and Transfer

Implement `SubscriptionRegistry` and `matches_replaces` for a bounded
RFC 6665 SUBSCRIBE/NOTIFY profile plus RFC 3515 REFER (implicit `refer`
subscription) and RFC 3891 Replaces matching. Input is a dialog identity,
an event-package name, subscription states, and Replaces identifiers.
Focus: composite keys, owned registry storage, state transitions, and exact
identifier matching. Never open sockets, parse a full SIP message, or
execute a transfer.

## Requirements

### Protocol roles (RFC 6665 / RFC 3515 / RFC 3891 teaching subset)

- A subscription is identified by a **dialog** (`Call-ID`, From-tag,
  To-tag) plus an **event package** string (`presence`, `refer`, …).
  Keep `DialogId` separate from `SubscriptionKey` so Replaces matching
  does not depend on an event name.
- RFC 6665 subscription states in this lab are `pending`, `active`, and
  `terminated`.
- A successful SUBSCRIBE (teaching `200`) creates the subscription; it
  does **not** by itself set the current event state. An immediate
  NOTIFY after that acceptance carries the initial state (often
  `pending` → `active` for presence).
- Allowed NOTIFY transitions for this teaching profile are only:
  `pending` → `pending`, `pending` → `active`, `pending` → `terminated`,
  `active` → `active`, and `active` → `terminated`. There is no
  transition out of `terminated`, and `active` → `pending` is invalid.
- Later NOTIFY messages may refresh (`active` → `active`) or terminate
  (`pending`/`active` → `terminated`). Once `terminated`, further
  NOTIFY updates for that key are invalid.
- REFER (RFC 3515) installs an implicit subscription to the `refer`
  event package on the same dialog keying rules. Progress and outcome
  are delivered by NOTIFY bodies of type `message/sipfrag` (for example
  a provisional `100 Trying` fragment, then a success `200 OK`
  fragment). This exercise models those outcomes as `SubscriptionState`
  updates on the `refer` key — it does not parse sipfrag text.
- RFC 3891 Replaces identifies a **target dialog** by Call-ID, to-tag,
  and from-tag. Matching is exact and case-sensitive on all three
  strings for the bounded fixtures.

### `SubscriptionRegistry::subscribe`

- Register `key` with the supplied `initial` state when the key is new.
- `initial` may be `pending` or `active`, but not `terminated`. A
  `terminated` initial value returns `false` and does not create an
  entry.
- Return `true` on successful registration.
- Return `false` when the same dialog+event key is already registered
  (duplicate subscribe: leave the existing entry unchanged).

### `SubscriptionRegistry::apply_notify`

- Look up `key`. If it is unknown, return `std::nullopt`.
- If the transition from the stored state to `next` is invalid for this
  teaching profile, return `std::nullopt` and leave storage unchanged.
- On a valid transition, store `next` and return that state.

### `matches_replaces`

- Return `true` only when `dialog.call_id == target.call_id`,
  `dialog.to_tag == target.to_tag`, and
  `dialog.from_tag == target.from_tag` (exact, case-sensitive).
- Any single-field mismatch is `false`.

### Out of scope

Building or sending SUBSCRIBE/NOTIFY/REFER requests, parsing
`message/sipfrag` bodies, network timers, and actually replacing or
bridging media sessions are **not** part of this exercise.

## How to start

1. Read the public contract in `starter.hpp` (`DialogId`,
   `SubscriptionKey`, `SubscriptionRegistry`, `Replaces`,
   `matches_replaces`).
2. Implement the registry methods and matcher in `starter.cpp`; keep
   `Solution::run` as a render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a
   finished solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day13` after wiring a real
   check.

## Acceptance fixtures

Use dialog `D1` unless a fixture names another:

- `call_id = "call-1"`
- `from_tag = "from-a"`
- `to_tag = "to-b"`

### Happy path — presence subscribe then immediate NOTIFY

- **Given** a new registry and key `{D1, event="presence"}`
- **When** `subscribe(key, pending)` then
  `apply_notify(key, active)` (immediate NOTIFY after SUBSCRIBE `200`)
- **Then** subscribe returns `true` and the NOTIFY returns `active`

### Happy path — presence refresh

- **Given** the active presence subscription above
- **When** `apply_notify(key, active)` again
- **Then** the result is `active` (refresh)

### Happy path — presence termination

- **Given** the active presence subscription
- **When** `apply_notify(key, terminated)`
- **Then** the result is `terminated`

### Happy path — REFER provisional then success

- **Given** a new registry and key `{D1, event="refer"}`
- **When** `subscribe(key, pending)`, then
  `apply_notify(key, active)` (sipfrag provisional context), then
  `apply_notify(key, terminated)` (sipfrag success / subscription ends)
- **Then** each step succeeds; final stored state is `terminated`

### Reject — unknown key

- **Given** an empty registry (or a key never subscribed)
- **When** `apply_notify({D1, "presence"}, active)` runs
- **Then** the result is `std::nullopt`

### Reject — duplicate subscribe

- **Given** `{D1, "presence"}` already registered
- **When** `subscribe` is called again for the same key
- **Then** it returns `false` and the stored state is unchanged

### Reject — invalid transition after terminated

- **Given** a subscription already in `terminated`
- **When** `apply_notify(key, active)` (or any non-terminal re-entry)
- **Then** the result is `std::nullopt` and storage stays `terminated`

### Reject — mismatched event on same dialog

- **Given** only `{D1, "presence"}` is registered
- **When** `apply_notify({D1, "refer"}, active)` runs
- **Then** the result is `std::nullopt` (event is part of the key)

### Happy path — exact Replaces match

- **Given** dialog `D1` and
  `Replaces{call_id="call-1", to_tag="to-b", from_tag="from-a"}`
- **When** `matches_replaces` runs
- **Then** the result is `true`

### Reject — near-miss Replaces identifiers

- **Given** dialog `D1` and a `Replaces` that differs in exactly one
  field (wrong `call_id`, wrong `to_tag`, or wrong `from_tag`, including
  case-only differences such as `"To-B"`)
- **When** `matches_replaces` runs
- **Then** the result is `false`
