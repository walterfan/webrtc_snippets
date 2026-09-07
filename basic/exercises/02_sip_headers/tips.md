# Day 02 tips

## Relevant knowledge

SIP header fields are `Name: Value` lines terminated by CRLF (RFC 3261
Section 7.3). Several names may repeat; `Via` is the usual teaching example.
This exercise does **not** implement header folding, comma-combined lists, or
case-insensitive name canonicalization.

C++ technique: walk the block with `string_view`, split on `\r\n`, split each
line on the first `:`, trim `' '` / `'\t'` from both sides, and store owned
`std::string` name/value pairs in a `std::vector<Header>` so the result does
not alias the input.

`parse_headers` is a public namespace function, not a private `Solution`
method. `run()` and the acceptance tests call it directly.

## Exercise contract

- **Input:** `std::string_view` of CRLF-delimited `Name: Value` lines.
- **Output:** `std::vector<Header>` in input order. Duplicate names are kept
  as separate entries.
- **Line rule:** a line without `:` is rejected: stop parsing and return the
  headers collected so far. A final line that lacks `\r\n` is ignored.
- **Trailing CRLF:** after the last header's `\r\n` the remainder is empty
  and the loop ends. An extra blank line (`\r\n` with no `:`) also stops the
  parse; it does not produce an empty header.
- **Ownership:** each `Header` owns copies of name and value.
- **Fixture:** the nine-header INVITE block in `run()` (two `Via` lines,
  then From, To, Call-ID, CSeq, Contact, Content-Type, Content-Length: 142).
  Expected count is **9**. Diagnostic text must say 9, not 8.

## Solution steps

1. Keep a `string_view` cursor over the remaining block.
2. While the cursor is not empty, find the next `\r\n`. Stop if it is
   missing (no implicit last line).
3. On the line, find the first `:`. Stop if it is missing (reject that line).
4. Trim the name and value, then `emplace_back` owned strings.
5. Advance the cursor past the CRLF.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| Two `Via` lines | two entries, same name, original order |
| Nine-header fixture | `size() == 9`, last value `"142"` |
| `Via: a\r\nNotAHeader\r\nVia: b\r\n` | one header (`Via` / `a`); parse stops |
| Line with spaces around `:` | name and value trimmed |
| Missing final CRLF on the last line | that line is not emitted |
| Collapsing duplicate names into one map entry | contract violation |

Common mistakes: using `\n` alone; erasing the second `Via`; returning
`string_view`s into the input; advertising 8 headers while asserting 9.

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day02
./build/basic/cpp_rtc_practice --run 02
```

`run()` must print `Parsed 9 headers` and return 0. A colon-less line must
not appear in the result.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Implement `parse_headers` for the stated CRLF-delimited teaching subset.
- Preserve input order and duplicate field names; reject a line that lacks `:`.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <string>
#include <string_view>
#include <vector>
namespace practice::day02 {

struct Header {
  std::string name;
  std::string value;
};

[[nodiscard]] std::vector<Header> parse_headers(std::string_view block);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day02
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day02 {

const char *sip =
  "Via: SIP/2.0/UDP a\r\n"
  "Via: SIP/2.0/TCP b\r\n"
  "From: \"Alice\" <sip:alice@atlanta.com>;tag=9fxced76sl\r\n"
  "To: Bob <sip:bob@biloxi.com>\r\n"
  "Call-ID: 2xQU9V7rfyb5uhAwH1s83d;1234567890\r\n"
  "CSeq: 314159 INVITE\r\n"
  "Contact: <sip:alice@atlanta.com>\r\n"
  "Content-Type: application/sdp\r\n"
  "Content-Length: 142\r\n";

// TODO(day02): Implement parse_headers and preserve duplicate header order.
int Solution::run(std::ostream &out) {
  out << "--- day 02 ---\n";
  auto headers = parse_headers(sip);

  out << "Parsed " << headers.size() << " headers\n";
  for (const auto &header : headers) {
    out << "Header: " << header.name << " = " << header.value << "\n";
  }
  if (headers.size() != 9) {
    out << "Expected 9 headers, got " << headers.size() << "\n";
    return 1;
  }
  int ret = is_eq(headers.size(), 9);
  ret += is_eq(headers[0].name, "Via");
  ret += is_eq(headers[0].value, "SIP/2.0/UDP a");
  ret += is_eq(headers[1].name, "Via");
  ret += is_eq(headers[1].value, "SIP/2.0/TCP b");
  ret += is_eq(headers[2].name, "From");
  ret += is_eq(headers[2].value, "\"Alice\" <sip:alice@atlanta.com>;tag=9fxced76sl");
  ret += is_eq(headers[3].name, "To");
  ret += is_eq(headers[3].value, "Bob <sip:bob@biloxi.com>");
  ret += is_eq(headers[4].name, "Call-ID");
  ret += is_eq(headers[4].value, "2xQU9V7rfyb5uhAwH1s83d;1234567890");
  ret += is_eq(headers[5].name, "CSeq");
  ret += is_eq(headers[5].value, "314159 INVITE");
  ret += is_eq(headers[6].name, "Contact");
  ret += is_eq(headers[6].value, "<sip:alice@atlanta.com>");
  ret += is_eq(headers[7].name, "Content-Type");
  ret += is_eq(headers[7].value, "application/sdp");
  ret += is_eq(headers[8].name, "Content-Length");
  ret += is_eq(headers[8].value, "142");
  return ret;
}


std::vector<Header> parse_headers(std::string_view block) {
  std::vector<Header> headers;
  std::string_view line = block;
  while (!line.empty()) {
    const std::size_t crlf = line.find("\r\n");
    if (crlf == std::string_view::npos) {
      break;
    }
    const std::string_view header = line.substr(0, crlf);
    const std::size_t colon = header.find(':');
    if (colon == std::string_view::npos) {
      break;
    }
    const std::string_view name = trim(header.substr(0, colon));
    const std::string_view value = trim(header.substr(colon + 1));
    headers.emplace_back(std::string(name), std::string(value));
    line = line.substr(crlf + 2);
  }

  return headers;
}

} // namespace practice::day02
```
