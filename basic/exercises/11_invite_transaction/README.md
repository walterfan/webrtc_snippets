# Day 11 — INVITE Transaction

Implement `transition` and `ack_owner` for a bounded INVITE client
transaction (RFC 3261 plus the RFC 6026 Accepted state). Input for
`transition` is a `State` and an `Event`; output is the next state or no
value for an invalid transition. Input for `ack_owner` is a response status
code; output says whether ACK handling belongs to the INVITE transaction or
to UA core, or no value when the code is not final. Focus: scoped enums,
`switch` or a transition table, and explicit invalid transitions. Never open
sockets, run retransmission clocks, or model the full proxy/server machine.

## Requirements

### States (teaching subset)

| State | Meaning |
| --- | --- |
| `calling` | INVITE sent; waiting for the first response. |
| `proceeding` | At least one provisional (`1xx`) has been received. |
| `completed` | A non-2xx final (`3xx`–`6xx`) has been received; the transaction still owns ACK. |
| `accepted` | A `2xx` has been received (RFC 6026). ACK generation belongs to UA core, not this transaction. |
| `terminated` | The transaction is finished; no further events are accepted. |

`accepted` is a **transaction** state. It is not the same thing as a
Confirmed Dialog at the dialog layer.

### Meaningful events by state

Event classes in this lab:

- `provisional` — any `1xx`
- `success_2xx` — any `2xx`
- `failure_3xx_6xx` — any `3xx`–`6xx`
- `timeout` — teaching stand-in for Timer B / H / M expiry (no wall clock)
- `transport_error` — send/receive failure on the transaction transport
- `ack` — ACK observed by the **transaction** (non-2xx path only)

Documented transitions (anything else returns `std::nullopt`):

| From | Event | To |
| --- | --- | --- |
| `calling` | `provisional` | `proceeding` |
| `calling` | `success_2xx` | `accepted` |
| `calling` | `failure_3xx_6xx` | `completed` |
| `calling` | `timeout` | `terminated` |
| `calling` | `transport_error` | `terminated` |
| `proceeding` | `provisional` | `proceeding` |
| `proceeding` | `success_2xx` | `accepted` |
| `proceeding` | `failure_3xx_6xx` | `completed` |
| `proceeding` | `timeout` | `terminated` |
| `proceeding` | `transport_error` | `terminated` |
| `completed` | `ack` | `terminated` |
| `completed` | `failure_3xx_6xx` | `completed` |
| `completed` | `timeout` | `terminated` |
| `completed` | `transport_error` | `terminated` |
| `accepted` | `success_2xx` | `accepted` |
| `accepted` | `timeout` | `terminated` |
| `accepted` | `transport_error` | `terminated` |
| `terminated` | *(any)* | *(none)* |

In particular:

- `ack` is valid only from `completed` (non-2xx ACK owned by the transaction).
- In `accepted`, retransmitted `2xx` responses are absorbed; `ack` is **not**
  a transaction event there (UA core owns that ACK).
- `terminated` rejects every event.

### ACK ownership (`ack_owner`)

- For a **2xx** status (`200`–`299`), return `AckOwner::ua_core` — UA core
  generates ACK; the INVITE transaction does not consume that ACK as a
  transition event.
- For a **non-2xx final** status (`300`–`699`), return
  `AckOwner::invite_transaction` — the transaction sends/matches ACK and
  moves `completed` → `terminated` on `ack`.
- For **non-final** codes (`100`–`199`) and any other out-of-range value,
  return `std::nullopt`.

Do not fold ACK ownership into `transition`; keep the two functions separate.

### CANCEL (separate transaction)

CANCEL is a **different** transaction. Do not add a `cancel` event to this
INVITE machine. When the UAS terminates the INVITE after a successful CANCEL,
the INVITE side typically receives `487 Request Terminated`, which is a
`failure_3xx_6xx` event on the INVITE transaction (`calling` /
`proceeding` → `completed`). Model that 487 on the INVITE path only; never
merge CANCEL into the INVITE state enum.

### Out of scope

Sockets, real timers, proxy/server INVITE machines, dialog Confirmed state,
and full message construction are not part of this exercise.

## How to start

1. Read the public contract in `starter.hpp` (`State`, `Event`, `transition`,
   `AckOwner`, `ack_owner`).
2. Implement both targets in `starter.cpp`; keep `Solution::run` as a render
   helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day11` after wiring a real check.

## Acceptance fixtures

### Happy path — calling to proceeding

- **Given** `State::calling` and `Event::provisional`
- **When** `transition` runs
- **Then** the next state is `proceeding`

### Happy path — proceeding absorbs further provisionals

- **Given** `State::proceeding` and `Event::provisional`
- **When** `transition` runs
- **Then** the state remains `proceeding`

### Happy path — success enters Accepted

- **Given** `calling` or `proceeding` and `Event::success_2xx`
- **When** `transition` runs
- **Then** the next state is `accepted`

### Happy path — failure enters Completed

- **Given** `calling` or `proceeding` and `Event::failure_3xx_6xx`
- **When** `transition` runs
- **Then** the next state is `completed`

### Happy path — non-2xx ACK terminates

- **Given** `State::completed` and `Event::ack`
- **When** `transition` runs
- **Then** the next state is `terminated`

### Happy path — Accepted absorbs 2xx retransmissions

- **Given** `State::accepted` and `Event::success_2xx`
- **When** `transition` runs
- **Then** the state remains `accepted`

### Happy path — timeout / transport error from early states

- **Given** `calling` or `proceeding` and `timeout` or `transport_error`
- **When** `transition` runs
- **Then** the next state is `terminated`

### Reject — ACK in Accepted is not a transaction event

- **Given** `State::accepted` and `Event::ack`
- **When** `transition` runs
- **Then** the result is empty

### Reject — every event in Terminated

- **Given** `State::terminated` and any `Event`
- **When** `transition` runs
- **Then** the result is empty

### Happy path — 2xx ACK ownership is UA core

- **Given** status `200`
- **When** `ack_owner` runs
- **Then** the result is `AckOwner::ua_core`

### Happy path — non-2xx ACK ownership is the INVITE transaction

- **Given** status `487` (or another `3xx`–`6xx`)
- **When** `ack_owner` runs
- **Then** the result is `AckOwner::invite_transaction`

### Reject — non-final status has no ACK owner

- **Given** status `180`
- **When** `ack_owner` runs
- **Then** the result is empty

### Happy path — CANCEL stays separate; 487 lands on INVITE

- **Given** an INVITE in `proceeding` after a peer CANCEL elsewhere
- **When** the INVITE receives `487` as `failure_3xx_6xx`
- **Then** `transition` yields `completed`, and CANCEL was never an INVITE
  event
