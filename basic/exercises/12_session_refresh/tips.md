# Day 12 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Validate positivity before Min-SE

Check that `offer.interval` is strictly positive **before** comparing it to
`min_se`. A zero or negative duration is a different failure class
(`invalid_interval`) from “below the floor” (`too_small`). Mixing those
checks can turn a bad duration into a false 422-shaped result.

## 2. Return typed failure, not a status string

Represent rejection as `SessionTimerFailure` inside the `SessionTimerResult`
variant. For a below-`Min-SE` offer, set `kind` to `too_small` and put the
required floor in `required_min` (teaching stand-in for 422). Prefer
returning structured data over inventing SIP response text in this function.

## 3. Make the refresher default explicit

When `offer.refresher` is empty, assign `Refresher::uac` on the success
path. When it is set, copy that role unchanged. Do not leave an
“unspecified” refresher on `SessionTimer` — the success type always carries
a concrete `Refresher`.

## 4. Half-interval math and method / glare rules

Compare fake `elapsed` to `timer.interval / 2` with ordinary
`std::chrono::seconds` integer division — no clocks, no sleep. For methods,
decide from `RefreshMethod` plus the early/confirmed flag: UPDATE is in
scope for early dialogs here; re-INVITE is not. Treat `491` as glare that
needs backoff, not an immediate same-message retry. Self-check: omitted
refresher → UAC; `30s` vs `Min-SE 90s` → `too_small`; `900s` of `1800s` is
due; early re-INVITE is false; only `491` is glare.

## Relevant knowledge

RFC 4028 `Session-Expires` is a **signaling** session lifetime. It is not
RTP keepalive and not a NAT binding refresh. `Min-SE` is the minimum
acceptable interval. An offer below that floor is a typed `too_small`
failure (422-shaped teaching data) that carries the required minimum. Zero
or negative intervals are `invalid_interval` — that is not a floor problem.

When the offer omits a refresher, this profile defaults to
`Refresher::uac`. The refresher becomes due at **half** the negotiated
interval using the caller-supplied fake `elapsed` duration.

RFC 3311 UPDATE may refresh an early dialog in this bounded profile.
re-INVITE is allowed only on a confirmed dialog (`early_dialog == false`).
Status `491 Request Pending` is glare: back off and retry later. Other
codes, including `422` and `500`, are not glare.

This exercise does not sleep, read `system_clock` / `steady_clock`, open
sockets, or construct UPDATE / re-INVITE messages.

C++ technique: `std::variant<SessionTimer, SessionTimerFailure>` for the
negotiate result; `std::optional<Refresher>` on the offer;
`std::chrono::seconds` integer division for the half-interval check.

## Exercise contract

- **`negotiate_session_timer(offer, min_se)`:**
  - `offer.interval <= 0s` →
    `SessionTimerFailure{invalid_interval, required_min=0s}`.
  - positive interval but `offer.interval < min_se` →
    `SessionTimerFailure{too_small, required_min=min_se}`.
  - otherwise `SessionTimer{offer.interval, refresher}` where refresher is
    `offer.refresher` or `Refresher::uac` when omitted.
- **`refresh_due(timer, elapsed)`:** `true` iff
  `elapsed >= timer.interval / 2` (integer `seconds` arithmetic).
- **`refresh_method_allowed(method, early_dialog)`:** `update` is always
  allowed; `reinvite` is allowed only when `early_dialog == false`.
- **`is_glare_retry(code)`:** `true` only for `491`.
- No wall clock, sleep, or network I/O.

## Solution steps

1. Reject non-positive `offer.interval` as `invalid_interval` with
   `required_min == 0s`.
2. Compare a positive interval to `min_se`. Below the floor → `too_small`
   with `required_min == min_se`.
3. On success, copy the interval and `value_or(Refresher::uac)`.
4. `refresh_due`: integer half-interval comparison on the fake duration.
5. `refresh_method_allowed`: UPDATE always; re-INVITE only when confirmed.
6. `is_glare_retry`: `code == 491`.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| `{1800s, nullopt}` vs `Min-SE 90s` | success, refresher `uac` |
| `{1800s, uas}` vs `90s` | success, refresher `uas` |
| `{30s, nullopt}` vs `90s` | `too_small`, `required_min=90s` |
| `{0s, …}` | `invalid_interval`, `required_min=0s` |
| `{-1s, …}` | `invalid_interval`, `required_min=0s` |
| timer `1800s`, elapsed `900s` | `refresh_due` true |
| timer `1800s`, elapsed `899s` | false |
| `update` + early | true |
| `reinvite` + early | false |
| `reinvite` + confirmed | true |
| `491` | glare true |
| `200`, `422`, `500` | glare false |

Common mistakes: treating `0s` as `too_small`; leaving refresher
unspecified; using `sleep` or `steady_clock`; comparing elapsed to the
full interval; treating every `4xx` as glare.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Negotiate Session-Expires against Min-SE; omit refresher → UAC.
- too_small (422-shaped) vs invalid_interval for non-positive offers.
- refresh_due at half interval; UPDATE early OK; re-INVITE confirmed only.
- Classify 491 as glare retry. No sleep or wall-clock reads.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <chrono>
#include <optional>
#include <variant>

namespace practice::day12 {

enum class Refresher { uac, uas };
enum class RefreshMethod { update, reinvite };

struct SessionTimerOffer {
  std::chrono::seconds interval;
  std::optional<Refresher> refresher;
};

struct SessionTimer {
  std::chrono::seconds interval;
  Refresher refresher;
};

enum class SessionTimerError { too_small, invalid_interval };

struct SessionTimerFailure {
  SessionTimerError kind;
  std::chrono::seconds required_min;
};

using SessionTimerResult =
    std::variant<SessionTimer, SessionTimerFailure>;

[[nodiscard]] SessionTimerResult
negotiate_session_timer(const SessionTimerOffer &offer,
                        std::chrono::seconds min_se);

[[nodiscard]] bool
refresh_due(const SessionTimer &timer,
            std::chrono::seconds elapsed);

[[nodiscard]] bool
refresh_method_allowed(RefreshMethod method, bool early_dialog);

[[nodiscard]] bool
is_glare_retry(unsigned short response_code);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day12
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day12 {
namespace {

[[nodiscard]] bool is_failure(const SessionTimerResult &result,
                              SessionTimerError kind,
                              std::chrono::seconds required_min) {
  const auto *failure = std::get_if<SessionTimerFailure>(&result);
  return failure != nullptr && failure->kind == kind &&
         failure->required_min == required_min;
}

} // namespace

SessionTimerResult
negotiate_session_timer(const SessionTimerOffer &offer,
                        std::chrono::seconds min_se) {
  if (offer.interval <= std::chrono::seconds{0}) {
    return SessionTimerFailure{SessionTimerError::invalid_interval,
                               std::chrono::seconds{0}};
  }
  if (offer.interval < min_se) {
    return SessionTimerFailure{SessionTimerError::too_small, min_se};
  }
  return SessionTimer{offer.interval,
                      offer.refresher.value_or(Refresher::uac)};
}

bool refresh_due(const SessionTimer &timer, std::chrono::seconds elapsed) {
  return elapsed >= timer.interval / 2;
}

bool refresh_method_allowed(RefreshMethod method, bool early_dialog) {
  if (method == RefreshMethod::update) {
    return true;
  }
  return !early_dialog;
}

bool is_glare_retry(unsigned short response_code) {
  return response_code == 491;
}

int Solution::run(std::ostream &out) {
  out << "--- day 12 ---\n";
  int ret = 0;
  const auto min_se = std::chrono::seconds{90};

  const auto omitted = negotiate_session_timer(
      SessionTimerOffer{std::chrono::seconds{1800}, std::nullopt}, min_se);
  ret += is_ok(std::holds_alternative<SessionTimer>(omitted));
  if (const auto *timer = std::get_if<SessionTimer>(&omitted)) {
    ret += is_eq(timer->interval.count(), 1800);
    ret += is_ok(timer->refresher == Refresher::uac);
  }

  const auto uas = negotiate_session_timer(
      SessionTimerOffer{std::chrono::seconds{1800}, Refresher::uas}, min_se);
  ret += is_ok(std::holds_alternative<SessionTimer>(uas));
  if (const auto *timer = std::get_if<SessionTimer>(&uas)) {
    ret += is_ok(timer->refresher == Refresher::uas);
  }

  ret += is_ok(is_failure(
      negotiate_session_timer(
          SessionTimerOffer{std::chrono::seconds{30}, std::nullopt}, min_se),
      SessionTimerError::too_small, std::chrono::seconds{90}));
  ret += is_ok(is_failure(
      negotiate_session_timer(
          SessionTimerOffer{std::chrono::seconds{0}, std::nullopt}, min_se),
      SessionTimerError::invalid_interval, std::chrono::seconds{0}));
  ret += is_ok(is_failure(
      negotiate_session_timer(
          SessionTimerOffer{std::chrono::seconds{-1}, std::nullopt}, min_se),
      SessionTimerError::invalid_interval, std::chrono::seconds{0}));

  const SessionTimer half{std::chrono::seconds{1800}, Refresher::uac};
  ret += is_ok(refresh_due(half, std::chrono::seconds{900}));
  ret += is_ok(!refresh_due(half, std::chrono::seconds{899}));

  ret += is_ok(refresh_method_allowed(RefreshMethod::update, true));
  ret += is_ok(refresh_method_allowed(RefreshMethod::update, false));
  ret += is_ok(!refresh_method_allowed(RefreshMethod::reinvite, true));
  ret += is_ok(refresh_method_allowed(RefreshMethod::reinvite, false));

  ret += is_ok(is_glare_retry(491));
  ret += is_ok(!is_glare_retry(500));
  ret += is_ok(!is_glare_retry(200));
  ret += is_ok(!is_glare_retry(422));
  return ret;
}

} // namespace practice::day12
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day12
./build/basic/cpp_rtc_practice --run 12
```

`run()` must return 0. Omitted refresher is UAC; `30s` vs `Min-SE 90s` is
`too_small`; `0s` / `-1s` are `invalid_interval` with `required_min=0s`;
`900s` of `1800s` is due and `899s` is not; early re-INVITE is false; only
`491` is glare. Do not sleep or read a wall clock.
