# Day 07 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Separate the lookup key from the binding key

Map the work around two stable identities before you pick containers:

1. The AOR string is the outer lookup key (`sip:alice@atlanta.com`).
2. Inside one AOR, the Contact URI is the binding key (desktop vs mobile vs
   browser).

Refresh and `expires=0` both target one URI under one AOR. Wildcard removal
targets the whole AOR and should not walk the `contacts` vector.

## 2. Own the bindings; decide order at lookup time

Store each `ContactBinding` as owned values (`std::string` / `std::vector`),
not views into the caller's `RegisterUpdate`. A nested map or map-of-vectors
is enough for teaching; prefer something that makes "find AOR, then find URI"
obvious.

Return a fresh `std::vector<ContactBinding>` from `lookup`. Sort that result
with a deterministic policy (higher `q_value` first; URI ascending on ties)
so callers never depend on insertion order.

## 3. Empty AOR, zero expiry, and cross-AOR isolation

Reject an empty AOR in `apply` without mutating any stored map. Treat
`expires == 0` as deletion of that URI only — do not confuse it with
wildcard clear. When the same URI is updated twice with positive expiry,
replace the previous binding fields rather than appending a duplicate.

Prove isolation with a second AOR: applying Bob's Contacts must leave Alice's
lookup unchanged, and Alice's wildcard clear must leave Bob alone.

## 4. Self-check before you declare done

Confirm that a three-Contact register, a desktop refresh, a mobile
`expires=0`, and a wildcard clear produce the README fixture results in order.
Confirm `lookup` never returns a Contact that was removed, never aliases
internal storage, and returns an empty vector for unknown AORs.

## Relevant knowledge

RFC 3261 REGISTER binds Contacts to an Address-of-Record (AOR). The AOR
(`sip:alice@atlanta.com`) is the stable lookup key. Each Contact URI is the
current device binding under that AOR. `expires > 0` creates or refreshes
one URI; `expires == 0` removes that URI only; a teaching `Contact: *`
wildcard (`wildcard_remove == true`) clears the whole AOR.

RFC 5626 Outbound and RFC 5627 GRUU are later binding-metadata topics, not
part of this store. Digest `401` / `407` challenges belong to Day 08; do
not compute Digest credentials, open sockets, or resolve DNS.

C++ technique: store owned `std::string` / value bindings in a nested map
(AOR → URI → `ContactBinding`). `lookup` copies into a fresh vector and
sorts it. Do not return references into the store.

## Exercise contract

- **Public API:** `RegistrationStore::apply(const RegisterUpdate &)` and
  `lookup(std::string_view) const`.
- **`apply`:** empty AOR → `false`, store unchanged. Otherwise apply the
  update and return `true`.
- **Positive expiry:** create or replace the binding for that AOR + URI
  (`expires` and `q_value` may change).
- **`expires == 0`:** delete that URI only. Missing URI is a no-op.
- **`wildcard_remove`:** ignore `contacts` and erase every binding for the
  AOR. Do not walk the vector.
- **`lookup`:** owned bindings for that AOR, higher `q_value` first, URI
  ascending on ties. Unknown or empty AOR → empty vector.
- **Isolation:** updates under Bob must not change Alice, and Alice's
  wildcard must not touch Bob.
- **Header storage:** the learner `starter.hpp` has no private members.
  The reference header adds an owned nested map; add equivalent private
  state in your header. Do not use process-global storage.

## Solution steps

1. Reject an empty AOR before touching any container.
2. On wildcard, erase the AOR key and return.
3. For each Contact: `expires == 0` erases that URI; otherwise insert or
   assign the owned binding.
4. Drop an AOR entry that becomes empty.
5. On lookup, copy the inner map into a vector and sort by the documented
   `q` / URI policy.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| desktop/mobile/browser with `q` 100, 50, 10 | three bindings, that order |
| refresh desktop `expires` / `q` | same URI, new fields; count stays 3 |
| mobile `expires=0` | desktop and browser remain |
| `wildcard_remove` for Alice | Alice empty; Bob unchanged |
| empty AOR | `apply` is `false`, no mutation |
| unknown AOR `lookup` | empty vector |
| two applies of the same URI | one binding, last fields win |
| returning a pointer into the map | contract violation |

Common mistakes: treating `expires == 0` as a wildcard; walking `contacts`
during wildcard clear; sorting only by URI; appending a duplicate URI on
refresh.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.
The reference header adds private owned storage that the learner skeleton
omits; the public `apply` / `lookup` signatures stay unchanged.

### `starter.hpp`

```cpp
/*
Requirements:
- Maintain Contact bindings keyed by AOR, with URI as the per-AOR binding key.
- Positive expires create/replace; expires==0 removes one Contact; wildcard
  clears the AOR. Lookups return owned bindings in deterministic q/URI order.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace practice::day07 {

struct ContactBinding {
  std::string uri;
  unsigned expires{};
  int q_value{};
};

struct RegisterUpdate {
  std::string aor;
  std::vector<ContactBinding> contacts;
  bool wildcard_remove{};
};

class RegistrationStore {
public:
  bool apply(const RegisterUpdate &update);
  [[nodiscard]] std::vector<ContactBinding>
  lookup(std::string_view aor) const;

private:
  std::map<std::string, std::map<std::string, ContactBinding>, std::less<>>
      bindings_;
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day07
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <algorithm>

namespace practice::day07 {

bool RegistrationStore::apply(const RegisterUpdate &update) {
  if (update.aor.empty()) {
    return false;
  }
  if (update.wildcard_remove) {
    bindings_.erase(update.aor);
    return true;
  }

  auto &by_uri = bindings_[update.aor];
  for (const auto &contact : update.contacts) {
    if (contact.expires == 0) {
      by_uri.erase(contact.uri);
      continue;
    }
    by_uri[contact.uri] = ContactBinding{contact.uri, contact.expires,
                                         contact.q_value};
  }
  if (by_uri.empty()) {
    bindings_.erase(update.aor);
  }
  return true;
}

std::vector<ContactBinding>
RegistrationStore::lookup(std::string_view aor) const {
  const auto found = bindings_.find(aor);
  if (found == bindings_.end()) {
    return {};
  }
  std::vector<ContactBinding> bindings;
  bindings.reserve(found->second.size());
  for (const auto &[uri, binding] : found->second) {
    bindings.push_back(binding);
  }
  std::sort(bindings.begin(), bindings.end(),
            [](const ContactBinding &left, const ContactBinding &right) {
              if (left.q_value != right.q_value) {
                return left.q_value > right.q_value;
              }
              return left.uri < right.uri;
            });
  return bindings;
}

int Solution::run(std::ostream &out) {
  out << "--- day 07 ---\n";
  RegistrationStore store;
  int ret = 0;

  const std::string alice = "sip:alice@atlanta.com";
  const std::string bob = "sip:bob@biloxi.com";
  const ContactBinding desktop{"sip:alice@pc.atlanta.com", 3600, 100};
  const ContactBinding mobile{"sip:alice@mobile.atlanta.com", 1800, 50};
  const ContactBinding browser{"sip:alice@browser.atlanta.com", 600, 10};

  ret += is_ok(!store.apply(RegisterUpdate{{}, {desktop}, false}));
  ret += is_ok(store.lookup(alice).empty());

  ret += is_ok(store.apply(RegisterUpdate{alice, {desktop, mobile, browser},
                                          false}));
  auto bindings = store.lookup(alice);
  ret += is_eq(bindings.size(), 3U);
  if (bindings.size() == 3) {
    ret += is_eq(bindings[0].uri, desktop.uri);
    ret += is_eq(bindings[1].uri, mobile.uri);
    ret += is_eq(bindings[2].uri, browser.uri);
    ret += is_eq(bindings[0].q_value, 100);
  }

  ret += is_ok(store.apply(
      RegisterUpdate{alice, {{desktop.uri, 7200, 90}}, false}));
  bindings = store.lookup(alice);
  ret += is_eq(bindings.size(), 3U);
  if (bindings.size() == 3) {
    ret += is_eq(bindings[0].expires, 7200U);
    ret += is_eq(bindings[0].q_value, 90);
    ret += is_eq(bindings[1].uri, mobile.uri);
    ret += is_eq(bindings[2].uri, browser.uri);
  }

  ret += is_ok(
      store.apply(RegisterUpdate{alice, {{mobile.uri, 0, 50}}, false}));
  bindings = store.lookup(alice);
  ret += is_eq(bindings.size(), 2U);
  if (bindings.size() == 2) {
    ret += is_eq(bindings[0].uri, desktop.uri);
    ret += is_eq(bindings[1].uri, browser.uri);
  }

  ret += is_ok(store.apply(
      RegisterUpdate{bob, {{"sip:bob@office.biloxi.com", 3600, 10}}, false}));
  ret += is_eq(store.lookup(bob).size(), 1U);
  ret += is_eq(store.lookup(alice).size(), 2U);

  ret += is_ok(store.apply(RegisterUpdate{alice, {}, true}));
  ret += is_ok(store.lookup(alice).empty());
  ret += is_eq(store.lookup(bob).size(), 1U);
  ret += is_ok(store.lookup("").empty());

  out << "alice after wildcard=" << store.lookup(alice).size()
      << " bob=" << store.lookup(bob).size() << '\n';
  return ret;
}

} // namespace practice::day07
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day07
./build/basic/cpp_rtc_practice --run 07
```

`run()` must return 0 for the README three-Contact, refresh, remove,
wildcard, empty-AOR, and isolation fixtures. `lookup` must never alias
internal storage. Do not implement Outbound, GRUU, or Digest.
