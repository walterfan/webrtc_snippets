# Day 16 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Validate length before you store a span

`from` is the only constructor path. If `bytes.size() < 12`, return
`std::nullopt` and do not index `bytes[0]`. Only after that check may
you call the private constructor with the caller’s span. The view then
stores that span as-is — header and payload together.

## 2. Subspan, do not copy

`header()` is `bytes_.first(12)`. `payload()` is `bytes_.subspan(12)`.
Both are views into the **same** storage the caller still owns. A
`vector` copy or a `string` would extend lifetime and fail the exercise.

## 3. Lifetime is the caller’s problem

`std::span` does not keep the buffer alive. If `from` is given a
temporary `vector` that dies at the end of the full expression, any
later `header()` / `payload()` use is dangling. Tests must keep the
backing array or vector in the same scope as the view.

## 4. Empty payload is valid

A 12-byte packet is a header with an empty payload, not a failure.
Self-check: 16-byte input → header 12 + payload 4, same `data()` base;
11-byte and empty inputs fail; a 12-byte input has `payload().empty()`.

## Relevant knowledge

RTP’s fixed header is 12 octets (RFC 3550). This day does not decode
fields; it only exposes two const spans. Day 15 already rejected bad
version / CSRC / extension packets.

C++ technique: `std::span<const std::byte>` is a pointer-plus-size.
Copying the span copies the view, not the bytes. `first` / `subspan`
are O(1) and stay inside the parent bounds if you pass 12 after a
`size() >= 12` check. No sockets, no crypto, no sleep.

## Exercise contract

- **Storage:** private `std::span<const std::byte> bytes_`. Caller owns
  the buffer. The view must not extend its lifetime.
- **`from(span)`:** `size < 12` → `nullopt`. Otherwise construct over
  the entire input span.
- **`header()`:** 12 bytes at offset 0.
- **`payload()`:** remaining bytes from offset 12 (may be empty).
- No V/P/X/CC checks here. No allocation. No `htons`.

## Solution steps

1. Reject short spans before any index or constructor call.
2. Store the caller span in the private constructor.
3. Return `first(12)` and `subspan(12)`.
4. In `run()`, keep fixture bytes in named storage for the life of the
   view.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| 16-byte caller buffer | header 12, payload 4, same base pointer |
| exactly 12 bytes | payload empty |
| size 0 or 11 | `nullopt`, no index |
| view copied | both copies alias the same caller bytes |
| `from(std::vector{...})` temporary | dangling after the statement |

Common mistakes: copying bytes into a `vector` member; returning
`span{local_array}`; treating `size == 12` as invalid; re-checking RTP
version here.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Create views only after validating the minimum header boundary.
- Return const spans into caller-owned storage without extending its lifetime.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstddef>
#include <optional>
#include <span>
namespace practice::day16 {

class RtpPacketView {
public:
  [[nodiscard]] static std::optional<RtpPacketView>
  from(std::span<const std::byte> bytes);
  [[nodiscard]] std::span<const std::byte> header() const;
  [[nodiscard]] std::span<const std::byte> payload() const;

private:
  explicit RtpPacketView(std::span<const std::byte> bytes);
  std::span<const std::byte> bytes_{};
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day16
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day16 {

RtpPacketView::RtpPacketView(std::span<const std::byte> bytes)
    : bytes_(bytes) {}

std::optional<RtpPacketView>
RtpPacketView::from(std::span<const std::byte> bytes) {
  if (bytes.size() < 12) {
    return std::nullopt;
  }
  return RtpPacketView{bytes};
}

std::span<const std::byte> RtpPacketView::header() const {
  return bytes_.first(12);
}

std::span<const std::byte> RtpPacketView::payload() const {
  return bytes_.subspan(12);
}

int Solution::run(std::ostream &out) {
  out << "--- day 16 ---\n";
  int ret = 0;

  const std::byte packet[16] = {
      std::byte{0x80}, std::byte{0x60}, std::byte{0x00}, std::byte{0x01},
      std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0xA0},
      std::byte{0x11}, std::byte{0x22}, std::byte{0x33}, std::byte{0x44},
      std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}, std::byte{0xDD}};

  const auto view = RtpPacketView::from(packet);
  ret += is_ok(view.has_value());
  if (view) {
    ret += is_ok(view->header().size() == 12);
    ret += is_ok(view->payload().size() == 4);
    ret += is_ok(view->header().data() == packet);
    ret += is_ok(view->payload().data() == packet + 12);
    ret += is_ok(view->payload()[0] == std::byte{0xAA});
    ret += is_ok(view->header().size() + view->payload().size() == 16);
  }

  const auto header_only = RtpPacketView::from(std::span{packet}.first(12));
  ret += is_ok(header_only.has_value() && header_only->payload().empty() &&
               header_only->header().size() == 12);

  ret += is_ok(!RtpPacketView::from(std::span<const std::byte>{}).has_value());
  ret += is_ok(!RtpPacketView::from(std::span{packet}.first(11)).has_value());
  return ret;
}

} // namespace practice::day16
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day16
./build/basic/cpp_rtc_practice --run 16
```

`run()` must return 0. Header and payload spans stay inside the caller
buffer. Empty and 11-byte spans fail. Do not copy the packet, open a
socket, or sleep.
