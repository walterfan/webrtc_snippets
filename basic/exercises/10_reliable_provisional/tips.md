# Day 10 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Three fields, then stop

Treat `RAck` as a fixed grammar: unsigned `rseq`, unsigned INVITE `cseq`,
then a method token. Split on horizontal whitespace (`' '` / `'\t'`), trim
each piece, and require **exactly three** non-empty fields. After the method,
the input must be fully consumed — trailing junk is a hard failure, not an
ignored suffix.

## 2. Checked numbers, owned method

Convert the two numeric fields with a wide intermediate (for example
`unsigned long` / `strtoul`-style scanning) and reject when:

- the scan leaves non-digit residue;
- the value is `0`;
- the value does not fit in `unsigned`.

Copy the method into `std::string` only on the success path so `RAck` does
not alias the caller's `string_view`.

## 3. Reliability is not “any 1xx”

`requires_prack` needs both a provisional in `101`–`199` **and**
`has_100rel == true`. Status `100` is never reliable here. An ordinary `180`
with `has_100rel == false` does not require PRACK. Do not confuse a peer
advertising `Supported: 100rel` in general with the per-response reliability
flag this function receives — they are not interchangeable inputs.

## 4. Match RSeq, INVITE CSeq, and expected method

`matches_prack` succeeds only when `rack.rseq` and `rack.invite_cseq` equal
the pending numbers **and** `rack.method` equals the separately supplied
`expected_method` argument. Do not invent the method from the `RAck` alone —
compare it to that caller-provided view. A single mismatched field fails the
whole check. Self-check: reliable `183` requires PRACK; `1 1 INVITE` matches
`rseq=1`, `invite_cseq=1`, `expected_method="INVITE"`; `1 1 INVITE EXTRA`, a
zero `rseq`, and a method of `ACK` do not.

## Relevant knowledge

RFC 3262 `100rel` marks a **non-100** provisional as reliable. The UAS
assigns an `RSeq`. The UAC answers with PRACK whose `RAck` echoes that
`RSeq`, the related INVITE `CSeq` number, and the INVITE method token.
`183 Session Progress` is the usual Early Media teaching case; treat it
like any other reliable non-100 provisional.

**`100 Trying` never requires PRACK**, even if a `100rel` flag is present.
An ordinary `180` with `has_100rel == false` also does not require PRACK.
`Supported: 100rel` in general is not the same input as the per-response
`has_100rel` flag.

This exercise does not schedule retransmits, parse SDP bodies on `183`,
construct SIP messages, or transport Early Media.

C++ technique: split `RAck` on `' '` / `'\t'`, require exactly three
non-empty tokens, convert the numbers with `std::from_chars` into
`unsigned`, and copy the method only on success.

## Exercise contract

- **`parse_rack(string_view)`:** grammar is `rseq` SP `invite_cseq` SP
  `method`. Horizontal whitespace around tokens is allowed. The input must
  be fully consumed. `rseq` and `invite_cseq` are unsigned decimals;
  reject `0`, non-digit residue, and overflow of `unsigned`. `method` is a
  non-empty token. Success returns an owned `RAck`.
- **`requires_prack(status, has_100rel)`:** `true` only for `101`–`199`
  when `has_100rel` is true. `100`, `>= 200`, and `has_100rel == false`
  are false.
- **`matches_prack(rseq, invite_cseq, expected_method, rack)`:** `true`
  only when all three fields equal the separately supplied pending
  identifiers. Do not infer the method from `RAck` alone.

## Solution steps

1. Tokenize on horizontal whitespace. Fail unless there are exactly three
   non-empty fields.
2. Convert both numbers with `from_chars`. Fail on residue, zero, or
   overflow. Copy the method into `std::string`.
3. `requires_prack`: `has_100rel && status >= 101 && status <= 199`.
4. `matches_prack`: compare `rseq`, `invite_cseq`, and `expected_method`.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| `183` + `has_100rel` | `requires_prack` true |
| `180` without `100rel` | false |
| `100` + `has_100rel` | false |
| `200` + `has_100rel` | false |
| `RAck` `1 1 INVITE` vs pending 1 / 1 / `INVITE` | match |
| pending `rseq == 2`, RAck `rseq == 1` | no match |
| pending INVITE CSeq `7`, RAck different | no match |
| `expected_method == INVITE`, RAck method `ACK` | no match |
| `1 1 INVITE EXTRA` | `parse_rack` → `nullopt` |
| `1 1` or `1 1 INVITE 1` | `nullopt` |
| `0 1 INVITE` or overflowing digits | `nullopt` |

Common mistakes: treating every `1xx` as reliable; accepting trailing junk;
using `std::stoul` without a full-consume check; matching a parsed `RAck`
against itself instead of `expected_method`.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Parse RAck as exactly rseq, invite CSeq, and method; own the method string.
- Require PRACK only for non-100 provisionals when 100rel is present.
- Match PRACK only when RSeq, INVITE CSeq, and expected_method all agree.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace practice::day10 {

struct RAck {
  unsigned rseq;
  unsigned invite_cseq;
  std::string method;
};

[[nodiscard]] std::optional<RAck>
parse_rack(std::string_view value);

[[nodiscard]] bool requires_prack(unsigned status_code,
                                  bool has_100rel);

[[nodiscard]] bool matches_prack(unsigned rseq,
                                 unsigned invite_cseq,
                                 std::string_view expected_method,
                                 const RAck &rack);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day10
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <charconv>
#include <system_error>
#include <vector>

namespace practice::day10 {
namespace {

[[nodiscard]] std::vector<std::string_view>
split_ws(std::string_view text) {
  std::vector<std::string_view> tokens;
  std::size_t i = 0;
  while (i < text.size()) {
    while (i < text.size() && (text[i] == ' ' || text[i] == '\t')) {
      ++i;
    }
    if (i >= text.size()) {
      break;
    }
    std::size_t j = i + 1;
    while (j < text.size() && text[j] != ' ' && text[j] != '\t') {
      ++j;
    }
    tokens.push_back(text.substr(i, j - i));
    i = j;
  }
  return tokens;
}

[[nodiscard]] std::optional<unsigned>
parse_nonzero_unsigned(std::string_view token) {
  if (token.empty()) {
    return std::nullopt;
  }
  unsigned value = 0;
  const auto *const begin = token.data();
  const auto *const end = begin + token.size();
  const auto [ptr, ec] = std::from_chars(begin, end, value);
  if (ec != std::errc{} || ptr != end || value == 0) {
    return std::nullopt;
  }
  return value;
}

} // namespace

std::optional<RAck> parse_rack(std::string_view value) {
  const auto tokens = split_ws(value);
  if (tokens.size() != 3) {
    return std::nullopt;
  }
  const auto rseq = parse_nonzero_unsigned(tokens[0]);
  const auto invite_cseq = parse_nonzero_unsigned(tokens[1]);
  if (!rseq || !invite_cseq || tokens[2].empty()) {
    return std::nullopt;
  }
  return RAck{*rseq, *invite_cseq, std::string(tokens[2])};
}

bool requires_prack(unsigned status_code, bool has_100rel) {
  return has_100rel && status_code >= 101 && status_code <= 199;
}

bool matches_prack(unsigned rseq, unsigned invite_cseq,
                   std::string_view expected_method, const RAck &rack) {
  return rack.rseq == rseq && rack.invite_cseq == invite_cseq &&
         rack.method == expected_method;
}

int Solution::run(std::ostream &out) {
  out << "--- day 10 ---\n";
  int ret = 0;

  ret += is_ok(requires_prack(183, true));
  ret += is_ok(!requires_prack(180, false));
  ret += is_ok(!requires_prack(100, true));
  ret += is_ok(!requires_prack(200, true));

  const auto rack = parse_rack("1 1 INVITE");
  ret += is_ok(rack.has_value());
  if (rack) {
    ret += is_ok(matches_prack(1, 1, "INVITE", *rack));
    ret += is_ok(!matches_prack(2, 1, "INVITE", *rack));
    ret += is_ok(!matches_prack(1, 7, "INVITE", *rack));
    ret += is_ok(!matches_prack(1, 1, "ACK", *rack));
    out << "rack=" << rack->rseq << " " << rack->invite_cseq << " "
        << rack->method << '\n';
  }

  ret += is_ok(!parse_rack("1 1 INVITE EXTRA").has_value());
  ret += is_ok(!parse_rack("1 1").has_value());
  ret += is_ok(!parse_rack("1 1 INVITE 1").has_value());
  ret += is_ok(!parse_rack("0 1 INVITE").has_value());
  ret += is_ok(!parse_rack("1 0 INVITE").has_value());
  ret += is_ok(!parse_rack("18446744073709551616 1 INVITE").has_value());
  return ret;
}

} // namespace practice::day10
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day10
./build/basic/cpp_rtc_practice --run 10
```

`run()` must return 0. Reliable `183` requires PRACK; `100` never does.
`1 1 INVITE` matches pending `1` / `1` / `INVITE`. Extra tokens, zero
`rseq`, and method `ACK` must fail. Do not start retransmission timers or
send Early Media.
