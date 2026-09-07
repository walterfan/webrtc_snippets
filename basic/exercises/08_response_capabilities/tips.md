# Day 08 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Build a constexpr table, not a forest of per-code branches

Represent each teaching status as one row: code, `string_view` phrase,
`ResponseClass`, and `final`. Prefer a `constexpr` array (or similar) and a
full-match lookup that returns `std::nullopt` when no row matches. Derive
category from the hundred-block boundaries (`1xx` … `6xx`) so provisional
versus final stays consistent with the class enum. Do not invent rows for
codes outside the README table.

## 2. Keep challenges and diagnostics as distinct rows

`401` and `407` share the client-error class but teach different hop roles —
keep separate phrases. Interval and dialog codes (`422`, `423`, `481`,
`487`, `488`, `491`) are also ordinary table entries: look them up the same
way as `200`. Confirm `183` is provisional (`final == false`) while every
`2xx`–`6xx` row in the table is final.

## 3. Compare option tags without mutating inputs

For `check_required_extensions`, walk `required` in order. Normalize each
tag for comparison (trim, case-fold) using temporary views or copies — never
edit the caller's vectors. A required tag is unsupported only when no
normalized `supported` entry matches. Preserve first-seen order in
`unsupported` and skip a tag you already recorded so duplicates do not
appear twice in the diagnostic list. `accepted` is true only when that list
stays empty.

## 4. Self-check before you declare done

Confirm `999` yields no `StatusInfo`. Confirm `Supported` listing a tag does
not satisfy a missing `Require` for that tag. Confirm Digest algorithms and
response-message construction remain out of scope — this exercise only
classifies codes and reports which required option tags were not supported.

## Relevant knowledge

SIP response classes are the hundred-blocks in RFC 3261: `1xx` provisional,
`2xx` success, `3xx` redirection, `4xx` client error, `5xx` server error,
`6xx` global failure. This teaching table is bounded; an unknown code such
as `999` is not invented. `401 Unauthorized` and
`407 Proxy Authentication Required` stay distinct rows. `420 Bad Extension`
is a table entry only — you do not build a wire `Unsupported` header.

`Require` / `Proxy-Require` are the tags a peer insists on. `Supported` is
what this side advertises. A required tag is accepted only when a
normalized supported tag matches. Listing a tag as supported does not mean
it was required.

**`constexpr status_info` location:** a `constexpr` definition must be
visible at its call site. Put the function body in `starter.hpp`. The
learner skeleton keeps a declaration only; a definition that lives only in
`starter.cpp` cannot be used as a constant expression from another
translation unit. `check_required_extensions` stays in `starter.cpp`.

Digest algorithms, response-message generation, and proxy routing are out
of scope.

## Exercise contract

- **`status_info(unsigned short)`:** `constexpr`, returns
  `optional<StatusInfo>` for the README table only.
- **Phrase, class, `final`:** phrase is the teaching string; class is the
  hundred-block; `final` is `false` only for `1xx`.
- **`check_required_extensions`:** walk `required` in order. Trim and
  case-fold copies for comparison; never mutate the caller's vectors.
  `unsupported` holds first-seen missing tags without duplicates.
  `accepted` is true only when that list is empty.
- **Ownership:** diagnostic strings are owned copies.

## Solution steps

1. Keep a `constexpr` row array of `{code, phrase}` in the header.
2. On a full-code match, derive `ResponseClass` from `code / 100` and set
   `final` from the hundred-block.
3. Return `nullopt` when no row matches.
4. For option tags, normalize both sides with trim + ASCII case-fold.
   Record each missing required tag once, in first-seen order.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| `180` | provisional, `final == false`, `Ringing` |
| `200` | success, `final == true`, `OK` |
| `183` | provisional, `final == false` |
| `401` vs `407` | both `client_error`, different phrases |
| `422` / `423` | interval errors, final `client_error` |
| `481` / `487` / `488` / `491` | dialog / media / glare codes |
| `503` | final `server_error` |
| `603` / `604` | final `global_failure` |
| `999` | `nullopt` |
| required `{100rel, timer}`, supported `{100rel, timer, path}` | accepted |
| required `{100rel, timer}`, supported `{100rel}` | `unsupported == {timer}` |
| required `{timer, 100rel, timer}`, supported `{100rel}` | `timer` once |
| `Supported` lists `timer` but `Require` does not | not a failure |

Common mistakes: collapsing `401` and `407`; inventing a phrase for `999`;
defining `status_info` only in the `.cpp`; mutating the input vectors;
emitting duplicate unsupported tags.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.
The reference header **defines** `status_info`; the learner header stays
declaration-only.

### `starter.hpp`

```cpp
/*
Requirements:
- Provide constexpr status_info for the bounded teaching status set only.
- Classify provisional vs final and keep 401 distinct from 407.
- check_required_extensions accepts only fully supported required tags and
  reports unsupported tags in first-seen order without duplicates.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace practice::day08 {

enum class ResponseClass {
  provisional,
  success,
  redirection,
  client_error,
  server_error,
  global_failure
};

struct StatusInfo {
  unsigned short code;
  std::string_view phrase;
  ResponseClass category;
  bool final;
};

[[nodiscard]] constexpr std::optional<StatusInfo>
status_info(unsigned short code) {
  struct Row {
    unsigned short code;
    std::string_view phrase;
  };
  constexpr Row kRows[] = {
      {100, "Trying"},
      {180, "Ringing"},
      {183, "Session Progress"},
      {200, "OK"},
      {202, "Accepted"},
      {301, "Moved Permanently"},
      {302, "Moved Temporarily"},
      {401, "Unauthorized"},
      {404, "Not Found"},
      {407, "Proxy Authentication Required"},
      {420, "Bad Extension"},
      {422, "Session Interval Too Small"},
      {423, "Interval Too Brief"},
      {480, "Temporarily Unavailable"},
      {481, "Call/Transaction Does Not Exist"},
      {486, "Busy Here"},
      {487, "Request Terminated"},
      {488, "Not Acceptable Here"},
      {491, "Request Pending"},
      {500, "Server Internal Error"},
      {503, "Service Unavailable"},
      {603, "Decline"},
      {604, "Does Not Exist Anywhere"},
  };
  for (const auto &row : kRows) {
    if (row.code != code) {
      continue;
    }
    ResponseClass category = ResponseClass::global_failure;
    switch (code / 100U) {
    case 1:
      category = ResponseClass::provisional;
      break;
    case 2:
      category = ResponseClass::success;
      break;
    case 3:
      category = ResponseClass::redirection;
      break;
    case 4:
      category = ResponseClass::client_error;
      break;
    case 5:
      category = ResponseClass::server_error;
      break;
    case 6:
      category = ResponseClass::global_failure;
      break;
    default:
      return std::nullopt;
    }
    return StatusInfo{row.code, row.phrase, category, code >= 200};
  }
  return std::nullopt;
}

struct CapabilityResult {
  bool accepted;
  std::vector<std::string> unsupported;
};

[[nodiscard]] CapabilityResult check_required_extensions(
    const std::vector<std::string> &required,
    const std::vector<std::string> &supported);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day08
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day08 {
namespace {

[[nodiscard]] char ascii_lower(char c) {
  if (c >= 'A' && c <= 'Z') {
    return static_cast<char>(c - 'A' + 'a');
  }
  return c;
}

[[nodiscard]] bool ascii_iequals(std::string_view left,
                                 std::string_view right) {
  if (left.size() != right.size()) {
    return false;
  }
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (ascii_lower(left[i]) != ascii_lower(right[i])) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] bool contains_tag(const std::vector<std::string> &tags,
                                std::string_view needle) {
  for (const auto &tag : tags) {
    if (ascii_iequals(trim(tag), needle)) {
      return true;
    }
  }
  return false;
}

} // namespace

CapabilityResult check_required_extensions(
    const std::vector<std::string> &required,
    const std::vector<std::string> &supported) {
  CapabilityResult result{true, {}};
  for (const auto &raw : required) {
    const auto tag = trim(raw);
    if (contains_tag(supported, tag)) {
      continue;
    }
    if (contains_tag(result.unsupported, tag)) {
      continue;
    }
    result.unsupported.emplace_back(tag);
    result.accepted = false;
  }
  return result;
}

int Solution::run(std::ostream &out) {
  out << "--- day 08 ---\n";
  int ret = 0;

  constexpr auto ringing = status_info(180);
  constexpr auto ok = status_info(200);
  constexpr auto unknown = status_info(999);
  ret += is_ok(ringing.has_value());
  if (ringing) {
    ret += is_ok(ringing->category == ResponseClass::provisional);
    ret += is_ok(!ringing->final);
    ret += is_eq(ringing->phrase, "Ringing");
  }
  ret += is_ok(ok.has_value());
  if (ok) {
    ret += is_ok(ok->category == ResponseClass::success);
    ret += is_ok(ok->final);
    ret += is_eq(ok->phrase, "OK");
  }
  ret += is_ok(!unknown.has_value());

  const auto challenge_uas = status_info(401);
  const auto challenge_proxy = status_info(407);
  ret += is_ok(challenge_uas && challenge_uas->final &&
               challenge_uas->category == ResponseClass::client_error &&
               challenge_uas->phrase == "Unauthorized");
  ret += is_ok(challenge_proxy &&
               challenge_proxy->phrase == "Proxy Authentication Required");

  ret += is_ok(status_info(422) &&
               status_info(422)->phrase == "Session Interval Too Small");
  ret += is_ok(status_info(423) &&
               status_info(423)->phrase == "Interval Too Brief");
  ret += is_ok(status_info(183) && !status_info(183)->final);
  ret += is_ok(status_info(481) &&
               status_info(481)->phrase == "Call/Transaction Does Not Exist");
  ret += is_ok(status_info(487) &&
               status_info(487)->phrase == "Request Terminated");
  ret += is_ok(status_info(488) &&
               status_info(488)->phrase == "Not Acceptable Here");
  ret += is_ok(status_info(491) &&
               status_info(491)->phrase == "Request Pending");
  ret += is_ok(status_info(503) &&
               status_info(503)->category == ResponseClass::server_error);
  ret += is_ok(status_info(603) &&
               status_info(603)->category == ResponseClass::global_failure);
  ret += is_ok(status_info(604) &&
               status_info(604)->category == ResponseClass::global_failure);

  const auto accepted = check_required_extensions(
      {"100rel", "timer"}, {"100rel", "timer", "path"});
  ret += is_ok(accepted.accepted);
  ret += is_ok(accepted.unsupported.empty());

  const auto missing =
      check_required_extensions({"100rel", "timer"}, {"100rel"});
  ret += is_ok(!missing.accepted);
  ret += is_eq(missing.unsupported.size(), 1U);
  if (!missing.unsupported.empty()) {
    ret += is_eq(missing.unsupported[0], "timer");
  }

  const auto dup =
      check_required_extensions({"timer", "100rel", "timer"}, {"100rel"});
  ret += is_ok(!dup.accepted);
  ret += is_eq(dup.unsupported.size(), 1U);
  if (!dup.unsupported.empty()) {
    ret += is_eq(dup.unsupported[0], "timer");
  }

  out << "180=" << (ringing ? ringing->phrase : "?")
      << " 200=" << (ok ? ok->phrase : "?") << '\n';
  return ret;
}

} // namespace practice::day08
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day08
./build/basic/cpp_rtc_practice --run 08
```

`run()` must return 0. `constexpr auto info = status_info(180);` must
compile only when the definition is visible at the call site. `999` must
be `nullopt`. Do not implement Digest or build a response message.
