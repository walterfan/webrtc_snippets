# Day 01 tips

## Relevant knowledge

A SIP request starts with a Request-Line: `Method SP Request-URI SP SIP-Version`
(RFC 3261 Section 7.1). This exercise is a teaching subset of that grammar:

- Fields are separated by a single ASCII space, not tabs or a variable run of
  whitespace.
- The only accepted version token is the exact string `SIP/2.0`.
- The returned `RequestLine` stores `string_view`s into the caller's input.
  The function must not copy the line into a temporary `std::string` and then
  return views of that temporary.

C++ technique: scan with `std::string_view::find`, slice with `substr`, and
report failure as `std::nullopt`. Do not use `from_chars` here; the version
token is compared as text.

## Exercise contract

- **Input:** one `std::string_view` holding a single ASCII request line.
- **Output:** `std::optional<RequestLine>` whose `method`, `uri`, and `version`
  views alias the input.
- **Success:** exactly three space-separated fields, a non-empty method, a
  non-empty Request-URI, and `version == "SIP/2.0"`.
- **Failure (`nullopt`):** blank/empty input; missing first or second space;
  empty method; empty URI (two adjacent spaces, as in `INVITE  SIP/2.0`); a
  fourth space-separated field; any version other than `SIP/2.0`.
- **Ownership:** views are valid only while the caller's input lives. The
  function does not allocate.

## Solution steps

1. Reject empty input immediately.
2. Find the first space. Reject if it is missing or at index 0 (empty method).
3. Find the second space. Reject if it is missing or immediately after the
   first space (empty URI).
4. Reject if a third space exists (extra fields).
5. Slice method, URI, and version. Reject an empty URI or a version that is
   not exactly `SIP/2.0`.
6. Return the three views.

## Edge cases and common mistakes

| Input | Result | Why |
| --- | --- | --- |
| `INVITE sip:bob@example.com SIP/2.0` | success | happy path |
| `` (empty) | `nullopt` | blank input |
| `INVITE SIP/2.0` | `nullopt` | missing URI field |
| `INVITE  SIP/2.0` | `nullopt` | empty URI between two spaces |
| `INVITE sip:bob@example.com SIP/2.0 extra` | `nullopt` | extra field |
| `INVITE sip:bob@example.com SIP/1.0` | `nullopt` | wrong version |
| ` INVITE sip:bob@example.com SIP/2.0` | `nullopt` | empty method |
| `INVITE sip:bob@example.com  SIP/2.0` | `nullopt` | extra space makes version not `SIP/2.0` |

Common mistakes: treating any whitespace as a delimiter; returning views into a
local `std::string`; accepting `SIP/2.0` with a trailing space as success;
forgetting that two consecutive spaces produce an empty URI.

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day01
./build/basic/cpp_rtc_practice --run 01
```

`run()` must return 0. The happy path must extract `INVITE`,
`sip:bob@example.com`, and `SIP/2.0`. The five listed failure cases must each
yield `nullopt`.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Implement `parse_request_line` without returning views into temporary storage.
- Accept exactly three space-separated fields and require `SIP/2.0` as the
version.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <optional>
#include <string_view>
namespace practice::day01 {

struct RequestLine {
  std::string_view method;
  std::string_view uri;
  std::string_view version;
};

[[nodiscard]] std::optional<RequestLine>
parse_request_line(std::string_view input);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day01
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day01 {

std::optional<RequestLine> parse_request_line(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }
  const std::size_t space1 = input.find(' ');
  if (space1 == std::string_view::npos || space1 == 0) {
    return std::nullopt;
  }
  const std::size_t space2 = input.find(' ', space1 + 1);
  if (space2 == std::string_view::npos || space2 == space1 + 1) {
    return std::nullopt;
  }
  if (input.find(' ', space2 + 1) != std::string_view::npos) {
    return std::nullopt;
  }
  const std::string_view method = input.substr(0, space1);
  const std::string_view uri = input.substr(space1 + 1, space2 - space1 - 1);
  const std::string_view version = input.substr(space2 + 1);
  if (method.empty() || uri.empty() || version != "SIP/2.0") {
    return std::nullopt;
  }
  return RequestLine{method, uri, version};
}

int Solution::run(std::ostream &out) {
  out << "--- day 01 ---\n";
  auto input = "INVITE sip:bob@example.com SIP/2.0";
  auto request_line = parse_request_line(input);
  if (!request_line) {
    out << "Failed to parse request line\n";
    return 1;
  }
  out << "Input: " << input << "\n";
  out << "Request line: " << request_line->method << " " << request_line->uri
      << " " << request_line->version << "\n";

  int ret = is_ok(request_line.has_value());
  ret += is_ok(request_line->method == "INVITE");
  ret += is_ok(request_line->uri == "sip:bob@example.com");
  ret += is_ok(request_line->version == "SIP/2.0");
  ret += is_ok(!parse_request_line("").has_value());
  ret += is_ok(!parse_request_line("INVITE SIP/2.0").has_value());
  ret += is_ok(!parse_request_line("INVITE  SIP/2.0").has_value());
  ret += is_ok(
      !parse_request_line("INVITE sip:bob@example.com SIP/1.0").has_value());
  ret += is_ok(
      !parse_request_line("INVITE sip:bob@example.com SIP/2.0 extra").has_value());
  return ret;
}

} // namespace practice::day01
```
