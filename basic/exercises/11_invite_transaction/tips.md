# Day 11 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Group status classes before transitions

Map response-shaped events to a few buckets first: provisional (`1xx`),
success (`2xx`), and failure (`3xx`–`6xx`). Then decide the next state from
the current `State` plus that bucket. Keep `timeout`, `transport_error`, and
`ack` as separate event kinds — they are not status codes. Prefer one
state-based dispatch with nested event handling, or an explicit table of
`(state, event) → next`, rather than scattering ad-hoc `if` chains.

## 2. Terminated rejects everything

Once the machine is in `terminated`, every event — including more
provisionals, `2xx`, `ack`, timeout, and transport errors — must yield
`std::nullopt`. Do not invent a self-loop on `terminated`. Treat “terminal”
as “no further transitions,” not “absorb all input.”

## 3. Separate ACK ownership from state transition

`ack_owner` answers *who generates/matches ACK for this status*, not *what
state comes next*. Use status ranges: `200`–`299` → `ua_core`, `300`–`699` →
`invite_transaction`, and anything non-final (or otherwise out of those
ranges) → `std::nullopt`. In `transition`, only the non-2xx path uses the
`ack` event (`completed` → `terminated`). Do not accept `ack` while in
`accepted`; UA core owns that ACK outside this function.

## 4. Accepted ≠ Confirmed Dialog; CANCEL stays separate

RFC 6026 `accepted` means the INVITE **transaction** saw a `2xx` and will
absorb further `2xx` until timeout/transport ends it. That is not dialog
Confirmed state — leave dialog lifetime alone. CANCEL is another
transaction: never add a cancel event to this enum. After CANCEL elsewhere,
the INVITE typically gets `487` as `failure_3xx_6xx` into `completed`.
Self-check: `calling` + provisional → `proceeding`; `proceeding` + `2xx` →
`accepted`; `completed` + `ack` → `terminated`; `accepted` + `ack` is empty;
`ack_owner(200)` is UA core; `ack_owner(180)` is empty.

## Relevant knowledge

RFC 3261 INVITE client transactions are not a single “wait for 200”
function. The first `1xx` moves `Calling` to `Proceeding`. A non-2xx final
response (`3xx`–`6xx`) moves to `Completed`, and **that** transaction owns
ACK. RFC 6026 adds `Accepted` for a `2xx`: the transaction absorbs further
`2xx` retransmissions, but UA core generates the ACK. `terminated` is a
dead end — no event, including `ack`, is valid there.

CANCEL is a **different** transaction. After a successful CANCEL, the INVITE
typically receives `487 Request Terminated`, which is just
`failure_3xx_6xx` on this machine. Do not add a cancel event.

This exercise does not start Timer B / H / M, open sockets, build SIP
messages, or model the proxy/server INVITE machine. `timeout` and
`transport_error` are caller-supplied event tokens.

C++ technique: scoped enums plus one `switch (state)` with a nested
`switch (event)`, or an explicit `(state, event) → next` table. Return
`std::nullopt` for every undocumented pair.

## Exercise contract

- **`transition(State, Event) → optional<State>`:** only the README table
  is valid. Anything else, including every event from `terminated`, is
  empty.
- **`calling` / `proceeding`:** `provisional` → `proceeding` (self-loop
  once already proceeding); `success_2xx` → `accepted`;
  `failure_3xx_6xx` → `completed`; `timeout` / `transport_error` →
  `terminated`. `ack` is invalid.
- **`completed`:** `ack` / `timeout` / `transport_error` → `terminated`;
  `failure_3xx_6xx` stays `completed`; `provisional` and `success_2xx`
  are empty.
- **`accepted`:** `success_2xx` stays `accepted`; `timeout` /
  `transport_error` → `terminated`; `ack` is empty (UA core owns that
  ACK). `provisional` and `failure_3xx_6xx` are empty.
- **`terminated`:** no self-loop. Every event is empty.
- **`ack_owner(status)`:** `200`–`299` → `AckOwner::ua_core`; `300`–`699`
  → `AckOwner::invite_transaction`; `100`–`199` and any other value →
  empty. Do not fold this into `transition`.
- **CANCEL:** never an `Event`. Model post-CANCEL `487` as
  `failure_3xx_6xx`.
- **`accepted` ≠ dialog Confirmed.** Transaction layer only.

## Solution steps

1. Dispatch on `state` first, then on `event`.
2. Implement the README table and return `nullopt` from every other arm,
   including the entire `terminated` state.
3. Keep `ack` only on the `completed` path.
4. Implement `ack_owner` as two inclusive status ranges. Leave
   provisionals and out-of-range codes empty.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| `calling` + `provisional` | `proceeding` |
| `proceeding` + `provisional` | `proceeding` |
| `calling` or `proceeding` + `success_2xx` | `accepted` |
| `calling` or `proceeding` + `failure_3xx_6xx` | `completed` |
| `calling` / `proceeding` + `timeout` or `transport_error` | `terminated` |
| `completed` + `ack` | `terminated` |
| `completed` + `failure_3xx_6xx` | `completed` |
| `accepted` + `success_2xx` | `accepted` |
| `accepted` + `ack` | empty |
| `terminated` + any event | empty |
| `calling` + `ack` | empty |
| `ack_owner(200)` | `ua_core` |
| `ack_owner(487)` | `invite_transaction` |
| `ack_owner(180)` | empty |
| `proceeding` + `failure_3xx_6xx` after a peer CANCEL | `completed` (no cancel event) |

Common mistakes: treating `accepted` as dialog Confirmed; accepting `ack`
in `accepted`; mapping `2xx` into `completed`; adding a `cancel` event;
self-looping `terminated`; mixing ACK ownership into `transition`.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Encode only the documented INVITE client transitions (incl. Accepted).
- Return no value for invalid or terminated-state events.
- ack_owner: 2xx → ua_core; 3xx–6xx → invite_transaction; else none.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>

namespace practice::day11 {

enum class State { calling, proceeding, completed, accepted, terminated };

enum class Event {
  provisional,
  success_2xx,
  failure_3xx_6xx,
  timeout,
  transport_error,
  ack
};

[[nodiscard]] std::optional<State> transition(State state, Event event);

enum class AckOwner { invite_transaction, ua_core };

[[nodiscard]] std::optional<AckOwner> ack_owner(unsigned short status_code);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day11
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day11 {

std::optional<State> transition(State state, Event event) {
  switch (state) {
  case State::calling:
    switch (event) {
    case Event::provisional:
      return State::proceeding;
    case Event::success_2xx:
      return State::accepted;
    case Event::failure_3xx_6xx:
      return State::completed;
    case Event::timeout:
    case Event::transport_error:
      return State::terminated;
    case Event::ack:
      return std::nullopt;
    }
    break;
  case State::proceeding:
    switch (event) {
    case Event::provisional:
      return State::proceeding;
    case Event::success_2xx:
      return State::accepted;
    case Event::failure_3xx_6xx:
      return State::completed;
    case Event::timeout:
    case Event::transport_error:
      return State::terminated;
    case Event::ack:
      return std::nullopt;
    }
    break;
  case State::completed:
    switch (event) {
    case Event::ack:
    case Event::timeout:
    case Event::transport_error:
      return State::terminated;
    case Event::failure_3xx_6xx:
      return State::completed;
    case Event::provisional:
    case Event::success_2xx:
      return std::nullopt;
    }
    break;
  case State::accepted:
    switch (event) {
    case Event::success_2xx:
      return State::accepted;
    case Event::timeout:
    case Event::transport_error:
      return State::terminated;
    case Event::provisional:
    case Event::failure_3xx_6xx:
    case Event::ack:
      return std::nullopt;
    }
    break;
  case State::terminated:
    return std::nullopt;
  }
  return std::nullopt;
}

std::optional<AckOwner> ack_owner(unsigned short status_code) {
  if (status_code >= 200 && status_code <= 299) {
    return AckOwner::ua_core;
  }
  if (status_code >= 300 && status_code <= 699) {
    return AckOwner::invite_transaction;
  }
  return std::nullopt;
}

int Solution::run(std::ostream &out) {
  out << "--- day 11 ---\n";
  int ret = 0;

  const auto calling_prov =
      transition(State::calling, Event::provisional);
  ret += is_ok(calling_prov.has_value() &&
               *calling_prov == State::proceeding);

  const auto proceeding_prov =
      transition(State::proceeding, Event::provisional);
  ret += is_ok(proceeding_prov.has_value() &&
               *proceeding_prov == State::proceeding);

  const auto calling_2xx = transition(State::calling, Event::success_2xx);
  ret += is_ok(calling_2xx.has_value() && *calling_2xx == State::accepted);
  const auto proceeding_2xx =
      transition(State::proceeding, Event::success_2xx);
  ret += is_ok(proceeding_2xx.has_value() &&
               *proceeding_2xx == State::accepted);

  const auto calling_fail =
      transition(State::calling, Event::failure_3xx_6xx);
  ret += is_ok(calling_fail.has_value() &&
               *calling_fail == State::completed);
  const auto proceeding_fail =
      transition(State::proceeding, Event::failure_3xx_6xx);
  ret += is_ok(proceeding_fail.has_value() &&
               *proceeding_fail == State::completed);

  const auto completed_ack = transition(State::completed, Event::ack);
  ret += is_ok(completed_ack.has_value() &&
               *completed_ack == State::terminated);

  const auto accepted_2xx = transition(State::accepted, Event::success_2xx);
  ret += is_ok(accepted_2xx.has_value() && *accepted_2xx == State::accepted);
  ret += is_ok(!transition(State::accepted, Event::ack).has_value());

  ret += is_ok(transition(State::calling, Event::timeout) ==
               std::optional<State>{State::terminated});
  ret += is_ok(transition(State::proceeding, Event::transport_error) ==
               std::optional<State>{State::terminated});
  ret += is_ok(!transition(State::calling, Event::ack).has_value());

  for (const Event event :
       {Event::provisional, Event::success_2xx, Event::failure_3xx_6xx,
        Event::timeout, Event::transport_error, Event::ack}) {
    ret += is_ok(!transition(State::terminated, event).has_value());
  }

  const auto owner_200 = ack_owner(200);
  ret += is_ok(owner_200.has_value() && *owner_200 == AckOwner::ua_core);
  const auto owner_487 = ack_owner(487);
  ret += is_ok(owner_487.has_value() &&
               *owner_487 == AckOwner::invite_transaction);
  ret += is_ok(!ack_owner(180).has_value());
  ret += is_ok(!ack_owner(99).has_value());

  const auto after_cancel =
      transition(State::proceeding, Event::failure_3xx_6xx);
  ret += is_ok(after_cancel.has_value() &&
               *after_cancel == State::completed);
  return ret;
}

} // namespace practice::day11
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day11
./build/basic/cpp_rtc_practice --run 11
```

`run()` must return 0. `calling` + provisional is `proceeding`; `2xx` is
`accepted`; non-2xx ACK is `completed` → `terminated`; `accepted` + `ack`
is empty; `ack_owner(200)` is UA core; `ack_owner(180)` is empty. After a
peer CANCEL, model `487` as `failure_3xx_6xx` only. Do not start
retransmission timers or open sockets.
