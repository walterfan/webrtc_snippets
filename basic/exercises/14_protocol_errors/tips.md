# Day 14 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Trim, then branch malformed vs out_of_range

Pipeline the Content-Length value in stages:

1. Trim leading/trailing `' '` and `'\t'` only.
2. Reject empty remainder as `malformed` / 400.
3. Reject a leading `+` / `-` as `malformed` / 400 (do not treat `-1`
   as a negative integer that becomes `out_of_range`).
4. Require every remaining character to be a decimal digit; any other
   character — including trailing junk after digits (`"42x"`) — is
   `malformed` / 400 (incomplete consumption).
5. Only after a complete all-digit token exists, convert and check the
   numeric range. Values outside 0…65535 are `out_of_range` / 400.

Use the two enum kinds; do not rely on `detail` alone to encode the split.

## 2. Checked conversion before narrowing

Prefer a wide intermediate (`unsigned long`, `unsigned long long`, or
`from_chars` into a wide type) and verify the entire token was consumed
before accepting a number. Reject as `malformed` when the digit scan did
not finish at the end of the token. Reject as `out_of_range` before
casting to `unsigned short` when the value is outside 0…65535. If you
use `std::stoul` / similar, catch conversion errors inside the function —
the public API must return `ProtocolError`, never throw.

## 3. One table for diagnose

Build a single source of truth (array, map, or compact dispatch) from
`ErrorKind` → response code for the README table, including **400** for
both `malformed` and `out_of_range`. Return a `ProtocolError` that owns
a copy of `detail`. Do not scatter magic numbers across helpers.

Remember: `auth_challenge` maps to **401**. **407** is the same
challenge *concept* at a proxy; it is documented in the README, not a
second enumerator.

## 4. Self-check before you declare done

Confirm `"42"` and `"  0042\t"` succeed; `""`, `"-1"`, `"oops"`, and
`"42x"` are `malformed` / 400; `"65536"` / `"999999"` are `out_of_range`
/ 400 without an escaped exception; each diagnose kind hits its table
code; and trust-boundary notes (PAI, Reason/History-Info, UUI, Identity,
SIPREC) stay reading context — no extra classes or network calls.

## Relevant knowledge

SIP parse failures at a teaching boundary are **typed**. `malformed` means
the token is not a complete unsigned decimal (empty, signed, non-digit, or
trailing junk). `out_of_range` means the token **is** a complete unsigned
decimal but the number is outside 0…65535. Both map to SIP `400 Bad
Request`. The kind field, not `detail`, carries that split.

`diagnose` is a table from `ErrorKind` to the normal response code: 400
(syntax and range), 483, 420, 401, 422, 423, 481, 488, 491, 503. `401` is
the UA/registrar challenge. `407` is the same challenge concept at a
proxy — same enumerator, different hop. Do not add a second `ErrorKind`.

PAI, Reason / History-Info, UUI, Identity, and SIPREC are reading notes
only. They are not extra APIs and they do not open sockets or verify
signatures.

C++ technique: trim `' '` / `'\t'` only; scan digits; convert with
`std::from_chars` into a wide unsigned type; never let `std::stoul`
exceptions escape the public function. `diagnose` takes `std::string` by
value so the returned `ProtocolError` owns `detail`.

## Exercise contract

- **`parse_content_length(string_view)`:**
  - Trim surrounding `' '` and `'\t'` only.
  - Success: all-digit token for an integer in **0…65535**, including
    leading zeroes (`"0042"` → `42`, `"0"` → `0`). Return
    `unsigned short` in the variant.
  - Empty after trim, leading `+` / `-`, any non-digit, or incomplete
    consumption (`"42x"`) → `ProtocolError{malformed, 400, …}`.
  - Complete all-digit token whose value is outside 0…65535, including
    overflowing digit strings → `ProtocolError{out_of_range, 400, …}`.
  - The public function does not throw.
- **`diagnose(kind, detail)`:** map `kind` to the README table code and
  return a `ProtocolError` that owns `detail`. `malformed` and
  `out_of_range` are both 400. `auth_challenge` is 401.
- No sockets, no PAI/UUI/SIPREC types, no Identity crypto.

## Solution steps

1. Trim spaces and tabs. Fail empty remainder as `malformed`.
2. Fail a leading sign as `malformed` before any numeric conversion.
3. Require every remaining character to be `'0'`–`'9'`. Any other
   character is `malformed`.
4. Convert with `from_chars` into `unsigned long long`. Overflow or a
   value above 65535 is `out_of_range` **before** narrowing to
   `unsigned short`.
5. `diagnose`: one switch or table from `ErrorKind` to the code; move
   `detail` into the returned object.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| `"42"` | `unsigned short{42}` |
| `"  0042\t"` | `42` |
| `"0"` / `"65535"` | `0` / `65535` |
| `""` / `"   "` | `malformed` / 400 |
| `"-1"` / `"+3"` / `"oops"` | `malformed` / 400 |
| `"42x"` | `malformed` / 400 |
| `"65536"` / `"999999"` | `out_of_range` / 400, no exception |
| `diagnose(malformed, "sample")` | code 400, detail `"sample"` |
| `diagnose(out_of_range, "sample")` | code 400, kind still `out_of_range` |
| `diagnose(auth_challenge, …)` | 401 (407 is proxy-equivalent, not an enum) |
| `too_many_hops` / `unsupported_extension` / `interval_too_small` / `interval_too_brief` / `dialog_not_found` / `media_not_acceptable` / `request_pending` / `server_unavailable` | 483 / 420 / 422 / 423 / 481 / 488 / 491 / 503 |

Common mistakes: classifying `"-1"` as `out_of_range`; accepting `"42x"`;
using `detail` alone to encode the split; letting `std::stoul` throw;
adding a `407` enumerator; implementing PAI or Identity.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- parse_content_length: trim spaces/tabs; accept 0..65535 with leading
  zeroes; empty/signed/non-digit/partial => malformed; numeric outside
  0..65535 => out_of_range; both use response_code 400; never leak
  exceptions from the public API.
- diagnose: map ErrorKind to 400 (malformed/out_of_range),
  483/420/401/422/423/481/488/491/503 (401 with 407 as proxy equivalent).
- No sockets; no PAI/UUI/SIPREC executable APIs.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <string>
#include <string_view>
#include <variant>

namespace practice::day14 {

enum class ErrorKind {
  malformed,
  out_of_range,
  too_many_hops,
  unsupported_extension,
  auth_challenge,
  interval_too_small,
  interval_too_brief,
  dialog_not_found,
  media_not_acceptable,
  request_pending,
  server_unavailable
};

struct ProtocolError {
  ErrorKind kind;
  unsigned short response_code;
  std::string detail;
};

using ContentLengthResult =
    std::variant<unsigned short, ProtocolError>;

[[nodiscard]] ContentLengthResult
parse_content_length(std::string_view value);

[[nodiscard]] ProtocolError
diagnose(ErrorKind kind, std::string detail);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day14
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <charconv>
#include <system_error>

namespace practice::day14 {
namespace {

[[nodiscard]] std::string_view trim_htab(std::string_view text) {
  while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
    text.remove_prefix(1);
  }
  while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) {
    text.remove_suffix(1);
  }
  return text;
}

[[nodiscard]] bool all_digits(std::string_view text) {
  if (text.empty()) {
    return false;
  }
  for (const char ch : text) {
    if (ch < '0' || ch > '9') {
      return false;
    }
  }
  return true;
}

[[nodiscard]] ProtocolError error(ErrorKind kind, std::string detail) {
  return diagnose(kind, std::move(detail));
}

[[nodiscard]] bool is_error(const ContentLengthResult &result, ErrorKind kind,
                            unsigned short code) {
  const auto *value = std::get_if<ProtocolError>(&result);
  return value != nullptr && value->kind == kind &&
         value->response_code == code;
}

} // namespace

ContentLengthResult parse_content_length(std::string_view value) {
  const auto token = trim_htab(value);
  if (token.empty()) {
    return error(ErrorKind::malformed, "empty");
  }
  if (token.front() == '+' || token.front() == '-') {
    return error(ErrorKind::malformed, "signed");
  }
  if (!all_digits(token)) {
    return error(ErrorKind::malformed, "non-digit");
  }

  unsigned long long parsed = 0;
  const auto *const begin = token.data();
  const auto *const end = begin + token.size();
  const auto [ptr, ec] = std::from_chars(begin, end, parsed);
  if (ec == std::errc::result_out_of_range || parsed > 65535ULL) {
    return error(ErrorKind::out_of_range, "out of range");
  }
  if (ec != std::errc{} || ptr != end) {
    return error(ErrorKind::malformed, "incomplete");
  }
  return static_cast<unsigned short>(parsed);
}

ProtocolError diagnose(ErrorKind kind, std::string detail) {
  unsigned short code = 400;
  switch (kind) {
  case ErrorKind::malformed:
  case ErrorKind::out_of_range:
    code = 400;
    break;
  case ErrorKind::too_many_hops:
    code = 483;
    break;
  case ErrorKind::unsupported_extension:
    code = 420;
    break;
  case ErrorKind::auth_challenge:
    code = 401;
    break;
  case ErrorKind::interval_too_small:
    code = 422;
    break;
  case ErrorKind::interval_too_brief:
    code = 423;
    break;
  case ErrorKind::dialog_not_found:
    code = 481;
    break;
  case ErrorKind::media_not_acceptable:
    code = 488;
    break;
  case ErrorKind::request_pending:
    code = 491;
    break;
  case ErrorKind::server_unavailable:
    code = 503;
    break;
  }
  return ProtocolError{kind, code, std::move(detail)};
}

int Solution::run(std::ostream &out) {
  out << "--- day 14 ---\n";
  int ret = 0;

  const auto plain = parse_content_length("42");
  ret += is_ok(std::holds_alternative<unsigned short>(plain));
  if (const auto *value = std::get_if<unsigned short>(&plain)) {
    ret += is_eq(*value, 42);
  }

  const auto padded = parse_content_length("  0042\t");
  ret += is_ok(std::holds_alternative<unsigned short>(padded));
  if (const auto *value = std::get_if<unsigned short>(&padded)) {
    ret += is_eq(*value, 42);
  }

  const auto zero = parse_content_length("0");
  const auto upper = parse_content_length("65535");
  ret += is_ok(std::holds_alternative<unsigned short>(zero) &&
               std::get<unsigned short>(zero) == 0);
  ret += is_ok(std::holds_alternative<unsigned short>(upper) &&
               std::get<unsigned short>(upper) == 65535);

  for (const auto *input : {"", "   ", "-1", "+3", "oops", "42x"}) {
    ret += is_ok(is_error(parse_content_length(input), ErrorKind::malformed,
                          400));
  }
  ret += is_ok(is_error(parse_content_length("65536"),
                        ErrorKind::out_of_range, 400));
  ret += is_ok(is_error(parse_content_length("999999"),
                        ErrorKind::out_of_range, 400));

  const auto sample = diagnose(ErrorKind::malformed, "sample");
  ret += is_eq(sample.response_code, 400);
  ret += is_ok(sample.detail == "sample");
  ret += is_eq(diagnose(ErrorKind::out_of_range, "sample").response_code, 400);
  ret += is_eq(diagnose(ErrorKind::too_many_hops, "sample").response_code, 483);
  ret += is_eq(
      diagnose(ErrorKind::unsupported_extension, "sample").response_code, 420);
  const auto challenge = diagnose(ErrorKind::auth_challenge, "sample");
  ret += is_eq(challenge.response_code, 401);
  ret += is_ok(challenge.kind == ErrorKind::auth_challenge);
  ret += is_eq(
      diagnose(ErrorKind::interval_too_small, "sample").response_code, 422);
  ret += is_eq(
      diagnose(ErrorKind::interval_too_brief, "sample").response_code, 423);
  ret += is_eq(diagnose(ErrorKind::dialog_not_found, "sample").response_code,
               481);
  ret += is_eq(
      diagnose(ErrorKind::media_not_acceptable, "sample").response_code, 488);
  ret += is_eq(diagnose(ErrorKind::request_pending, "sample").response_code,
               491);
  ret += is_eq(
      diagnose(ErrorKind::server_unavailable, "sample").response_code, 503);
  return ret;
}

} // namespace practice::day14
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day14
./build/basic/cpp_rtc_practice --run 14
```

`run()` must return 0. `"42"` and `"  0042\t"` succeed; `""`, `"-1"`,
`"oops"`, and `"42x"` are `malformed` / 400; `"65536"` is `out_of_range` /
400 with no escaped exception. `diagnose(auth_challenge, …)` is 401; 407
stays a documented proxy equivalent. Do not add PAI, Identity, or network
APIs.
