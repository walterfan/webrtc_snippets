# Day 13 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Composite key includes the event package

Do not key the registry on `DialogId` alone. Two subscriptions on the same
dialog with different event packages (`presence` vs `refer`) are distinct.
A nested map, a map keyed by a tuple you define, or an equivalent structure
works for teaching — as long as lookup requires dialog **and** event.

Keep `DialogId` as its own type so `matches_replaces` can compare dialog
identifiers without inventing a fake event name.

## 2. Copy keys into owned storage

`subscribe` and `apply_notify` take `const SubscriptionKey &`. Store owned
`std::string` copies (and owned `DialogId` fields) inside the registry so
entries outlive the caller's temporaries. Do not keep pointers or
`string_view`s into the argument.

## 3. Small explicit transition policy — not a free-form graph

Decide allowed `(current, next)` pairs up front for this lab's three
states. Reject unknown keys with `std::nullopt`. Reject illegal moves
(especially anything after `terminated`) the same way, without mutating
storage. Prefer a short allow-list or a tiny helper over scattering
ad-hoc `if` chains.

Do **not** treat a `200` response to SUBSCRIBE as the event state itself —
registration establishes the subscription; NOTIFY advances state. For
REFER, progress and outcome arrive as NOTIFY on the `refer` package
(sipfrag context in the README), not as a separate side channel in this
API.

## 4. Exact, case-sensitive Replaces matching

Compare `call_id`, `to_tag`, and `from_tag` with ordinary `std::string`
equality. A single near-miss field fails the whole match. Self-check:
presence subscribe → immediate `active` NOTIFY; refresh stays `active`;
terminate once; unknown/`refer`-on-presence-only keys are empty; exact
Replaces is true; `"to-b"` ≠ `"To-B"`.

## Relevant knowledge

RFC 6665 identifies a subscription by dialog (`Call-ID`, From-tag, To-tag)
**plus** an event package. `presence` and `refer` on the same dialog are
different keys. Teaching states are `pending`, `active`, and `terminated`.

A successful SUBSCRIBE (teaching `200`) **registers** the key; it does not
set the current event state. An immediate NOTIFY carries the first state
(often `pending` → `active` for presence). Later NOTIFY messages may
refresh (`active` → `active`) or terminate. There is no transition out of
`terminated`, and `active` → `pending` is invalid.

RFC 3515 REFER installs an implicit `refer` subscription on the same keying
rules. Progress and outcome arrive as NOTIFY (`message/sipfrag` in the
README). This exercise stores those outcomes as `SubscriptionState` on the
`refer` key — it does not parse sipfrag text.

RFC 3891 Replaces names a **target dialog** by Call-ID, to-tag, and
from-tag. Matching is exact and case-sensitive on all three strings.

This exercise does not build SUBSCRIBE/NOTIFY/REFER, parse sipfrag, start
timers, or replace/bridge media.

C++ technique: own every stored string; key the map by dialog fields plus
event; keep a tiny allow-list of `(current, next)` pairs.

## Exercise contract

- **`subscribe(key, initial)`:** register a **new** key. `initial` may be
  `pending` or `active`, not `terminated` (`false`, no entry). Duplicate
  dialog+event returns `false` and leaves the existing entry unchanged.
  Success returns `true` after copying owned strings into storage.
- **`apply_notify(key, next)`:** unknown key → `nullopt`. Illegal
  transition → `nullopt`, storage unchanged. Valid transition stores
  `next` and returns it.
- **Allowed NOTIFY transitions:** `pending` → `pending` / `active` /
  `terminated`; `active` → `active` / `terminated`. Nothing from
  `terminated`. `active` → `pending` is invalid.
- **`matches_replaces(dialog, target)`:** `true` only when
  `dialog.call_id == target.call_id`, `dialog.to_tag == target.to_tag`,
  and `dialog.from_tag == target.from_tag` (exact, case-sensitive).
- **Ownership:** stored keys outlive the caller's `SubscriptionKey`.
  `DialogId` stays independent of event so Replaces does not invent a
  package name.
- The learner `starter.hpp` has no private members. The reference header
  adds owned map storage; public signatures stay unchanged.

## Solution steps

1. Reject `initial == terminated` before touching storage.
2. Look up dialog+event. If present, `subscribe` returns `false`.
3. Otherwise copy the key fields and store `initial`.
4. `apply_notify`: fail on unknown key or a pair outside the allow-list;
   otherwise write `next` and return it.
5. `matches_replaces`: three exact string comparisons. Watch field order:
   `DialogId` is `call_id`, `from_tag`, `to_tag`; `Replaces` is `call_id`,
   `to_tag`, `from_tag`.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| `{D1, presence}` subscribe `pending`, NOTIFY `active` | `true`, then `active` |
| second NOTIFY `active` | `active` (refresh) |
| NOTIFY `terminated` | `terminated` |
| `{D1, refer}` pending → active → terminated | each step succeeds |
| NOTIFY on a key never subscribed | `nullopt` |
| duplicate subscribe | `false`, state unchanged |
| NOTIFY `active` after `terminated` | `nullopt`, stays `terminated` |
| `{D1, refer}` while only presence exists | `nullopt` |
| subscribe with `initial == terminated` | `false`, no entry |
| `active` → `pending` | `nullopt` |
| Replaces exact D1 tags | `true` |
| `to_tag == "To-B"` | `false` |

Common mistakes: keying only on `DialogId`; treating SUBSCRIBE `200` as
the event state; parsing sipfrag; case-folding Replaces tags; executing a
transfer; storing `string_view`s into the caller's key.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.
The reference header adds private owned storage that the learner skeleton
omits; the public `subscribe` / `apply_notify` / `matches_replaces`
signatures stay unchanged.

### `starter.hpp`

```cpp
/*
Requirements:
- Key subscriptions by DialogId + event package; own stored strings.
- subscribe registers a new key; apply_notify updates or rejects.
- matches_replaces: exact Call-ID / to-tag / from-tag equality.
- No sockets, sipfrag parsing, or transfer execution.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <map>
#include <optional>
#include <string>
#include <tuple>

namespace practice::day13 {

enum class SubscriptionState { pending, active, terminated };

struct DialogId {
  std::string call_id;
  std::string from_tag;
  std::string to_tag;
};

struct SubscriptionKey {
  DialogId dialog;
  std::string event;
};

class SubscriptionRegistry {
public:
  bool subscribe(const SubscriptionKey &key,
                 SubscriptionState initial);

  [[nodiscard]] std::optional<SubscriptionState>
  apply_notify(const SubscriptionKey &key,
               SubscriptionState next);

private:
  struct StoredKey {
    std::string call_id;
    std::string from_tag;
    std::string to_tag;
    std::string event;

    friend bool operator<(const StoredKey &left, const StoredKey &right) {
      return std::tie(left.call_id, left.from_tag, left.to_tag, left.event) <
             std::tie(right.call_id, right.from_tag, right.to_tag,
                      right.event);
    }
  };

  std::map<StoredKey, SubscriptionState> entries_;
};

struct Replaces {
  std::string call_id;
  std::string to_tag;
  std::string from_tag;
};

[[nodiscard]] bool
matches_replaces(const DialogId &dialog,
                 const Replaces &target);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day13
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day13 {
namespace {

[[nodiscard]] bool
notify_allowed(SubscriptionState current, SubscriptionState next) {
  if (current == SubscriptionState::terminated) {
    return false;
  }
  if (current == SubscriptionState::pending) {
    return next == SubscriptionState::pending ||
           next == SubscriptionState::active ||
           next == SubscriptionState::terminated;
  }
  return next == SubscriptionState::active ||
         next == SubscriptionState::terminated;
}

} // namespace

bool SubscriptionRegistry::subscribe(const SubscriptionKey &key,
                                     SubscriptionState initial) {
  if (initial == SubscriptionState::terminated) {
    return false;
  }
  const StoredKey stored{key.dialog.call_id, key.dialog.from_tag,
                         key.dialog.to_tag, key.event};
  if (entries_.contains(stored)) {
    return false;
  }
  entries_.emplace(stored, initial);
  return true;
}

std::optional<SubscriptionState>
SubscriptionRegistry::apply_notify(const SubscriptionKey &key,
                                   SubscriptionState next) {
  const StoredKey stored{key.dialog.call_id, key.dialog.from_tag,
                         key.dialog.to_tag, key.event};
  const auto found = entries_.find(stored);
  if (found == entries_.end()) {
    return std::nullopt;
  }
  if (!notify_allowed(found->second, next)) {
    return std::nullopt;
  }
  found->second = next;
  return next;
}

bool matches_replaces(const DialogId &dialog, const Replaces &target) {
  return dialog.call_id == target.call_id &&
         dialog.to_tag == target.to_tag &&
         dialog.from_tag == target.from_tag;
}

int Solution::run(std::ostream &out) {
  out << "--- day 13 ---\n";
  int ret = 0;
  const DialogId d1{"call-1", "from-a", "to-b"};
  const SubscriptionKey presence{d1, "presence"};
  const SubscriptionKey refer{d1, "refer"};

  SubscriptionRegistry registry;
  ret += is_ok(registry.subscribe(presence, SubscriptionState::pending));
  const auto became_active =
      registry.apply_notify(presence, SubscriptionState::active);
  ret += is_ok(became_active.has_value() &&
               *became_active == SubscriptionState::active);
  const auto refreshed =
      registry.apply_notify(presence, SubscriptionState::active);
  ret += is_ok(refreshed.has_value() &&
               *refreshed == SubscriptionState::active);
  const auto ended =
      registry.apply_notify(presence, SubscriptionState::terminated);
  ret += is_ok(ended.has_value() && *ended == SubscriptionState::terminated);
  ret += is_ok(
      !registry.apply_notify(presence, SubscriptionState::active).has_value());

  SubscriptionRegistry refer_store;
  ret += is_ok(refer_store.subscribe(refer, SubscriptionState::pending));
  ret += is_ok(
      refer_store.apply_notify(refer, SubscriptionState::active).has_value());
  const auto refer_done =
      refer_store.apply_notify(refer, SubscriptionState::terminated);
  ret += is_ok(refer_done.has_value() &&
               *refer_done == SubscriptionState::terminated);

  SubscriptionRegistry empty;
  ret += is_ok(
      !empty.apply_notify(presence, SubscriptionState::active).has_value());
  ret += is_ok(!registry.subscribe(presence, SubscriptionState::pending));
  ret += is_ok(
      !registry.apply_notify(refer, SubscriptionState::active).has_value());
  ret += is_ok(
      !empty.subscribe(presence, SubscriptionState::terminated));

  const auto after_active = [&] {
    SubscriptionRegistry store;
    store.subscribe(presence, SubscriptionState::active);
    return store.apply_notify(presence, SubscriptionState::pending);
  }();
  ret += is_ok(!after_active.has_value());

  const Replaces exact{"call-1", "to-b", "from-a"};
  ret += is_ok(matches_replaces(d1, exact));
  ret += is_ok(!matches_replaces(d1, Replaces{"call-9", "to-b", "from-a"}));
  ret += is_ok(!matches_replaces(d1, Replaces{"call-1", "To-B", "from-a"}));
  ret += is_ok(!matches_replaces(d1, Replaces{"call-1", "to-b", "From-A"}));
  return ret;
}

} // namespace practice::day13
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day13
./build/basic/cpp_rtc_practice --run 13
```

`run()` must return 0. Presence subscribe then immediate `active` NOTIFY
succeeds; refresh stays `active`; terminate once; further NOTIFY is empty.
`refer` is a separate key. Exact Replaces matches; `"To-B"` does not. Do
not parse sipfrag or execute a transfer.

When overlaying the answer, copy **both** fenced files. The learner header
has no storage; `starter.cpp` alone will not compile.
