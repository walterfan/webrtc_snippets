# Day 06 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Split the hop into three regions

Map the work separately before combining results:

1. Transport token after `SIP/2.0/` until the first space (or end of the
   version/transport field).
2. Sent-by token until the first `;` (host, optional `:port`).
3. Semicolon-separated parameters after that point.

Fail early if the `SIP/2.0/` prefix or the transport token is missing. Keep
the three regions as views until you know the parse succeeded.

## 2. Trim, then own

Trim horizontal whitespace (`' '` / `'\t'`) from each token and each
`name` / `value` before comparing or storing. Copy into `std::string` (or
`optional<string>`) only when building the returned `ViaHop` so the result
does not alias the caller's `string_view`.

## 3. Branch uniqueness, empty rport, and port overflow

Track whether `branch` has already been seen; a second occurrence is a hard
failure even if both values match. Distinguish three `rport` states:

- parameter absent;
- parameter present with no value (`rport` or `rport=`);
- parameter present with digits.

Parse the numeric form with a wide enough intermediate (for example
`unsigned long`) and reject before narrowing into `unsigned short` when the
value is outside 0…65535 or the digit scan did not consume the whole token.

## 4. Self-check before you declare done

Confirm that a successful `received=` updates only `ViaHop::received` and
leaves `sent_by` as the host written in the Via line. Confirm that an unknown
but syntactically valid parameter (for example `ttl=1`) is ignored, while a
broken token (empty name, stray `=`, non-decimal `rport`) still yields
`nullopt` — unknowns must not hide malformed syntax.

## Relevant knowledge

RFC 3261 Via parameters identify the hop that must receive the response.
RFC 3581 `rport` asks the next hop to send that response to the observed
source port. This exercise parses **one Via header value** (not the `Via:`
name) and never opens a socket, resolves DNS, validates TLS, or sends a
response. Transport names (`UDP`, `TCP`, `TLS`, `WS`, `WSS`) are data only.

C++ technique: keep `string_view` slices while tokenizing; trim `' '` /
`'\t'` before comparing; copy into owned `std::string` / `optional` fields
only on the success path. Parse numeric `rport` with `std::from_chars` into
a wide intermediate and reject before narrowing into `unsigned short`.

## Exercise contract

- **Input:** `std::string_view` beginning with the exact prefix `SIP/2.0/`.
- **Output:** `std::optional<ViaHop>`. Failure is `nullopt`.
- **Transport:** the token after `SIP/2.0/` until the first horizontal
  whitespace, mapped case-insensitively onto `Transport`.
- **Sent-by:** the next non-empty token until the first `;`. The optional
  `:port` stays inside `sent_by`; it is not a separate field.
- **Parameters:** semicolon-separated `name` or `name=value`. Names are
  case-insensitive. Trim name and value.
- **branch:** exactly one non-empty value. A second `branch`, even with the
  same text, is a hard failure.
- **received:** copy the observed address into `ViaHop::received`; do not
  replace `sent_by`. Absent or empty `received` leaves that optional empty.
- **rport:** absent → `rport_requested == false` and empty `rport`; present
  with no value (`rport` or `rport=`) → requested, empty port; present with
  a decimal value in **0…65535** → requested and that port.
- **Unknown parameters:** ignore only when `name` / `name=value` is
  syntactically valid. Empty name, stray `=`, or a broken numeric `rport`
  fail the whole parse.
- **Ownership:** every stored string is owned. The result must not alias the
  caller's `string_view`.

## Solution steps

1. Trim the whole value. Require the `SIP/2.0/` prefix.
2. Take the transport token, map it, and fail if it is unknown.
3. Take the sent-by token before `;`. Fail if it is empty.
4. Walk parameters. Track whether `branch` was already seen. Classify
   `received` and the three `rport` states. Ignore other valid tokens.
5. Fail if `branch` is missing or empty. Build an owned `ViaHop` only after
   every check succeeds.

## Edge cases and common mistakes

| Input | Result | Why |
| --- | --- | --- |
| UDP + `received` + `rport=5070` | success | happy path |
| `SIP/2.0/TLS proxy.example.com:5061;branch=…;rport` | success, empty `rport` | empty rport request |
| `Branch` / `Received` / `RPort` | same as lowercase | names are case-insensitive |
| missing `branch` | `nullopt` | required parameter |
| two `branch` parameters | `nullopt` | uniqueness |
| `rport=70000` | `nullopt` | outside 0…65535 |
| `rport=abc` or `rport=50x` | `nullopt` | non-decimal residue |
| `ttl=1` with a valid `branch` | success | unknown but well-formed |
| `;=foo` or `;= ` | `nullopt` | empty name / stray `=` |
| `received=192.0.2.1` | `sent_by` unchanged | received is a separate field |

Common mistakes: treating `WSS` as `WS`; replacing `sent_by` with
`received`; accepting a second `branch`; using `std::stoi` without a full
consume check; returning views into the input.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Parse one Via value beginning with SIP/2.0/<transport>.
- Require non-empty sent-by and exactly one non-empty branch.
- Treat parameter names as case-insensitive; honor empty vs numeric rport.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace practice::day06 {

enum class Transport { udp, tcp, tls, ws, wss };

struct ViaHop {
  Transport transport;
  std::string sent_by;
  std::string branch;
  std::optional<std::string> received;
  std::optional<unsigned short> rport;
  bool rport_requested{};
};

[[nodiscard]] std::optional<ViaHop>
parse_via_hop(std::string_view value);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day06
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <charconv>
#include <limits>
#include <system_error>

namespace practice::day06 {
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

[[nodiscard]] std::optional<Transport>
parse_transport(std::string_view token) {
  if (ascii_iequals(token, "UDP")) {
    return Transport::udp;
  }
  if (ascii_iequals(token, "TCP")) {
    return Transport::tcp;
  }
  if (ascii_iequals(token, "TLS")) {
    return Transport::tls;
  }
  if (ascii_iequals(token, "WSS")) {
    return Transport::wss;
  }
  if (ascii_iequals(token, "WS")) {
    return Transport::ws;
  }
  return std::nullopt;
}

[[nodiscard]] std::optional<unsigned short>
parse_port(std::string_view token) {
  if (token.empty()) {
    return std::nullopt;
  }
  unsigned long value = 0;
  const auto *const begin = token.data();
  const auto *const end = begin + token.size();
  const auto [ptr, ec] = std::from_chars(begin, end, value);
  if (ec != std::errc{} || ptr != end ||
      value > std::numeric_limits<unsigned short>::max()) {
    return std::nullopt;
  }
  return static_cast<unsigned short>(value);
}

} // namespace

std::optional<ViaHop> parse_via_hop(std::string_view value) {
  auto text = trim(value);
  constexpr std::string_view kPrefix = "SIP/2.0/";
  if (!text.starts_with(kPrefix)) {
    return std::nullopt;
  }
  text.remove_prefix(kPrefix.size());

  const auto transport_end = text.find_first_of(" \t");
  if (transport_end == std::string_view::npos || transport_end == 0) {
    return std::nullopt;
  }
  const auto transport = parse_transport(trim(text.substr(0, transport_end)));
  if (!transport) {
    return std::nullopt;
  }
  text.remove_prefix(transport_end);
  text = trim(text);
  if (text.empty()) {
    return std::nullopt;
  }

  const auto sent_by_end = text.find(';');
  const auto sent_by = trim(sent_by_end == std::string_view::npos
                                ? text
                                : text.substr(0, sent_by_end));
  if (sent_by.empty()) {
    return std::nullopt;
  }

  ViaHop hop{};
  hop.transport = *transport;
  hop.sent_by = std::string(sent_by);

  auto params = sent_by_end == std::string_view::npos
                    ? std::string_view{}
                    : text.substr(sent_by_end + 1);
  bool have_branch = false;
  while (!params.empty()) {
    const auto next = params.find(';');
    const auto raw = trim(params.substr(0, next));
    params = next == std::string_view::npos ? std::string_view{}
                                            : params.substr(next + 1);
    if (raw.empty()) {
      return std::nullopt;
    }
    const auto eq = raw.find('=');
    const auto name = trim(eq == std::string_view::npos ? raw
                                                       : raw.substr(0, eq));
    const auto pvalue =
        eq == std::string_view::npos ? std::string_view{}
                                     : trim(raw.substr(eq + 1));
    if (name.empty()) {
      return std::nullopt;
    }
    if (ascii_iequals(name, "branch")) {
      if (have_branch || pvalue.empty()) {
        return std::nullopt;
      }
      hop.branch = std::string(pvalue);
      have_branch = true;
    } else if (ascii_iequals(name, "received")) {
      if (!pvalue.empty()) {
        hop.received = std::string(pvalue);
      }
    } else if (ascii_iequals(name, "rport")) {
      hop.rport_requested = true;
      if (!pvalue.empty()) {
        const auto port = parse_port(pvalue);
        if (!port) {
          return std::nullopt;
        }
        hop.rport = port;
      }
    }
  }
  if (!have_branch) {
    return std::nullopt;
  }
  return hop;
}

int Solution::run(std::ostream &out) {
  out << "--- day 06 ---\n";
  int ret = 0;

  const auto udp = parse_via_hop(
      "SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bK-1;"
      "received=192.0.2.1;rport=5070");
  ret += is_ok(udp.has_value());
  if (udp) {
    ret += is_ok(udp->transport == Transport::udp);
    ret += is_eq(udp->sent_by, "pc33.atlanta.com");
    ret += is_eq(udp->branch, "z9hG4bK-1");
    ret += is_ok(udp->received.has_value() &&
                 *udp->received == "192.0.2.1");
    ret += is_ok(udp->rport.has_value() && *udp->rport == 5070);
    ret += is_ok(udp->rport_requested);
    out << "udp sent_by=" << udp->sent_by << " received=" << *udp->received
        << " rport=" << *udp->rport << '\n';
  }

  const auto tls = parse_via_hop(
      "SIP/2.0/TLS proxy.example.com:5061;branch=z9hG4bK776asdhds;rport");
  ret += is_ok(tls.has_value());
  if (tls) {
    ret += is_ok(tls->transport == Transport::tls);
    ret += is_eq(tls->sent_by, "proxy.example.com:5061");
    ret += is_eq(tls->branch, "z9hG4bK776asdhds");
    ret += is_ok(!tls->received.has_value());
    ret += is_ok(!tls->rport.has_value());
    ret += is_ok(tls->rport_requested);
  }

  const auto mixed = parse_via_hop(
      "SIP/2.0/UDP host.example;Branch=z9hG4bKabc;Received=198.51.100.9;"
      "RPort=5080");
  ret += is_ok(mixed.has_value());
  if (mixed) {
    ret += is_eq(mixed->branch, "z9hG4bKabc");
    ret += is_ok(mixed->received.has_value() &&
                 *mixed->received == "198.51.100.9");
    ret += is_ok(mixed->rport.has_value() && *mixed->rport == 5080);
    ret += is_ok(mixed->rport_requested);
  }

  const auto ignored = parse_via_hop(
      "SIP/2.0/UDP host.example;branch=z9hG4bK-1;ttl=1");
  ret += is_ok(ignored.has_value());
  ret += is_ok(!parse_via_hop("SIP/2.0/UDP host.example;received=192.0.2.1")
                    .has_value());
  ret += is_ok(!parse_via_hop("SIP/2.0/UDP host.example;branch=z9hG4bK-1;"
                             "branch=z9hG4bK-2")
                    .has_value());
  ret += is_ok(!parse_via_hop("SIP/2.0/UDP host.example;branch=z9hG4bK-1;"
                             "rport=70000")
                    .has_value());
  ret += is_ok(!parse_via_hop("SIP/2.0/UDP host.example;branch=z9hG4bK-1;"
                             "rport=50x")
                    .has_value());
  ret += is_ok(
      !parse_via_hop("SIP/2.0/UDP host.example;branch=z9hG4bK-1;=").has_value());
  return ret;
}

} // namespace practice::day06
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day06
./build/basic/cpp_rtc_practice --run 06
```

`run()` must return 0 after the README happy-path and reject fixtures. A
successful `received=` must leave `sent_by` unchanged. `ttl=1` must not hide
a missing or duplicate `branch`. No implementation may open a socket or
resolve the sent-by name.
