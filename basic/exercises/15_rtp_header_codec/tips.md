# Day 15 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Length first, then version, then the rest

Do not index `packet[0]` until `packet.size() >= 12`. A truncated buffer is
a hard failure, not a partial header. After the length check, read byte 0
and require `V == 2` before treating sequence, timestamp, or SSRC as valid
fields. Only then unpack marker, payload type, and the three multi-byte
integers.

## 2. Mask V/P/X/CC; do not confuse the first-byte layout

Byte 0 is one bitfield: `V` in bits 6–7, `P` in bit 5, `X` in bit 4, `CC`
in bits 0–3. Extract each with shifts and masks. `P` is observed so the
layout stays honest; this teaching `RtpHeader` does not store it, and
`encode_header` always writes `P = 0`.

## 3. Reject CSRC and extension; keep the 12-byte subset

`CC != 0` means a CSRC list follows the fixed header. `X == 1` means a
header extension follows that list. Both are **unsupported** here: return
`std::nullopt` even if the buffer is long enough to hold those extra words.
Bytes after the first 12 with `V=2`, `X=0`, `CC=0` are payload (or
padding) and are ignored. `encode_header` emits exactly 12 bytes:
`V=2`, `P=0`, `X=0`, `CC=0`.

## 4. Network byte order without socket helpers

Sequence is a 16-bit big-endian integer; timestamp and SSRC are 32-bit
big-endian integers. Shift and mask the bytes yourself. Do not call
`htons` / `ntohl` or include socket headers. Payload type is the low 7
bits of byte 1; mask it on both encode and decode. Self-check: a valid
12-byte V=2 header round-trips; 11-byte and empty spans fail before any
index; `CC=1` and `X=1` fail even at 12 bytes.

## Relevant knowledge

RFC 3550's first 12 octets are the only fields this exercise owns:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

`V` must be 2. Unsupported CSRC (`CC != 0`) and header-extension (`X == 1`)
packets are rejected. The teaching struct stores payload type, marker,
sequence, timestamp, and SSRC only.

C++ technique: `std::span<const std::byte>` for the input, `std::vector<
std::byte>` for the encoded 12 octets, `std::to_integer` when reading,
explicit shifts for big-endian integers. No sockets, SRTP, or sleep.

## Exercise contract

- **`decode_header(span<const byte>)`:**
  - If `packet.size() < 12`, return `nullopt` **before indexing any
    byte**.
  - Then read byte 0. Require `V == 2`. Reject `X == 1` and `CC != 0`.
  - `P` is parsed for layout only and is not stored.
  - Extra bytes after offset 12 are ignored.
  - Success returns `payload_type` (7 bits), `marker`, `sequence`,
    `timestamp`, and `ssrc` decoded from network byte order.
- **`encode_header(header)`:** exactly 12 bytes, `V=2 P=0 X=0 CC=0`,
  marker and 7-bit payload type in byte 1, then big-endian sequence,
  timestamp, and SSRC. Do not emit CSRC or extension words.
- No `htons` / sockets, no crypto, no sleep.

## Solution steps

1. Reject `size < 12` with no indexed read.
2. Split byte 0 into `V`, `P`, `X`, `CC`. Fail `V != 2`, `X == 1`, or
   `CC != 0`.
3. Split byte 1 into marker and 7-bit payload type.
4. Assemble sequence, timestamp, and SSRC with left shifts (big-endian).
5. Encode the inverse: write `0x80` in byte 0, mask PT, pack the three
   integers as big-endian bytes.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| 12-byte `V=2 P=0 X=0 CC=0` | fields extracted |
| those fields through `encode_header` | same 12 bytes (`P` forced to 0) |
| packet longer than 12 with `X=0 CC=0` | header decoded; extra ignored |
| size 0 or 11 | `nullopt`, no index |
| `V != 2` | `nullopt` |
| `CC != 0` (CSRC present) | `nullopt` even if length is enough |
| `X == 1` (extension present) | `nullopt` even if length is enough |
| `payload_type` above 127 on encode | stored/sent as the low 7 bits |

Common mistakes: reading `packet[0]` before the length check; treating
CSRCs or extensions as part of this subset; using host endianness;
calling `htons`; storing the full first byte as payload type.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Validate packet length and RTP version before indexing any byte.
- Encode/decode exactly the documented fixed-header subset with network byte
order.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace practice::day15 {

struct RtpHeader {
  std::uint8_t payload_type{};
  bool marker{};
  std::uint16_t sequence{};
  std::uint32_t timestamp{};
  std::uint32_t ssrc{};
};

[[nodiscard]] std::optional<RtpHeader>
decode_header(std::span<const std::byte> packet);
[[nodiscard]] std::vector<std::byte> encode_header(const RtpHeader &header);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day15
```

### `starter.cpp`

```cpp
#include "starter.hpp"

namespace practice::day15 {
namespace {

[[nodiscard]] std::uint8_t u8(std::byte value) {
  return std::to_integer<std::uint8_t>(value);
}

[[nodiscard]] bool same_header(const RtpHeader &left, const RtpHeader &right) {
  return left.payload_type == right.payload_type &&
         left.marker == right.marker && left.sequence == right.sequence &&
         left.timestamp == right.timestamp && left.ssrc == right.ssrc;
}

} // namespace

std::optional<RtpHeader> decode_header(std::span<const std::byte> packet) {
  if (packet.size() < 12) {
    return std::nullopt;
  }

  const auto first = u8(packet[0]);
  const auto version = static_cast<std::uint8_t>(first >> 6);
  [[maybe_unused]] const bool padding = (first & 0x20u) != 0;
  const bool extension = (first & 0x10u) != 0;
  const auto csrc_count = static_cast<std::uint8_t>(first & 0x0Fu);
  if (version != 2 || extension || csrc_count != 0) {
    return std::nullopt;
  }

  const auto second = u8(packet[1]);
  RtpHeader header;
  header.marker = (second & 0x80u) != 0;
  header.payload_type = static_cast<std::uint8_t>(second & 0x7Fu);
  header.sequence = static_cast<std::uint16_t>((u8(packet[2]) << 8) |
                                               u8(packet[3]));
  header.timestamp = (static_cast<std::uint32_t>(u8(packet[4])) << 24) |
                     (static_cast<std::uint32_t>(u8(packet[5])) << 16) |
                     (static_cast<std::uint32_t>(u8(packet[6])) << 8) |
                     static_cast<std::uint32_t>(u8(packet[7]));
  header.ssrc = (static_cast<std::uint32_t>(u8(packet[8])) << 24) |
                (static_cast<std::uint32_t>(u8(packet[9])) << 16) |
                (static_cast<std::uint32_t>(u8(packet[10])) << 8) |
                static_cast<std::uint32_t>(u8(packet[11]));
  return header;
}

std::vector<std::byte> encode_header(const RtpHeader &header) {
  std::vector<std::byte> packet(12, std::byte{0});
  packet[0] = std::byte{0x80};
  const auto payload_type =
      static_cast<std::uint8_t>(header.payload_type & 0x7Fu);
  packet[1] = std::byte{static_cast<std::uint8_t>(
      (header.marker ? 0x80u : 0x00u) | payload_type)};
  packet[2] = std::byte{static_cast<std::uint8_t>(header.sequence >> 8)};
  packet[3] = std::byte{static_cast<std::uint8_t>(header.sequence)};
  packet[4] = std::byte{static_cast<std::uint8_t>(header.timestamp >> 24)};
  packet[5] = std::byte{static_cast<std::uint8_t>(header.timestamp >> 16)};
  packet[6] = std::byte{static_cast<std::uint8_t>(header.timestamp >> 8)};
  packet[7] = std::byte{static_cast<std::uint8_t>(header.timestamp)};
  packet[8] = std::byte{static_cast<std::uint8_t>(header.ssrc >> 24)};
  packet[9] = std::byte{static_cast<std::uint8_t>(header.ssrc >> 16)};
  packet[10] = std::byte{static_cast<std::uint8_t>(header.ssrc >> 8)};
  packet[11] = std::byte{static_cast<std::uint8_t>(header.ssrc)};
  return packet;
}

int Solution::run(std::ostream &out) {
  out << "--- day 15 ---\n";
  int ret = 0;

  const RtpHeader sample{96, true, 1, 160, 0xAABBCCDDu};
  const auto encoded = encode_header(sample);
  ret += is_ok(encoded.size() == 12);
  ret += is_ok(encoded[0] == std::byte{0x80});
  ret += is_ok(encoded[1] == std::byte{0xE0});
  ret += is_ok(encoded[2] == std::byte{0x00});
  ret += is_ok(encoded[3] == std::byte{0x01});

  const auto decoded = decode_header(encoded);
  ret += is_ok(decoded.has_value() && same_header(*decoded, sample));

  const auto with_payload = [&] {
    auto packet = encoded;
    packet.push_back(std::byte{0xFF});
    return decode_header(packet);
  }();
  ret += is_ok(with_payload.has_value() &&
               same_header(*with_payload, sample));

  ret += is_ok(!decode_header(std::span<const std::byte>{}).has_value());
  const std::byte short_packet[11]{};
  ret += is_ok(!decode_header(short_packet).has_value());

  auto version1 = encoded;
  version1[0] = std::byte{0x40};
  ret += is_ok(!decode_header(version1).has_value());

  auto with_csrc = encoded;
  with_csrc[0] = std::byte{0x81};
  ret += is_ok(!decode_header(with_csrc).has_value());

  auto with_extension = encoded;
  with_extension[0] = std::byte{0x90};
  ret += is_ok(!decode_header(with_extension).has_value());

  const auto high_pt = encode_header(RtpHeader{200, false, 0, 0, 0});
  ret += is_ok(high_pt[1] == std::byte{0x48});
  return ret;
}

} // namespace practice::day15
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day15
./build/basic/cpp_rtc_practice --run 15
```

`run()` must return 0. A 12-byte V=2 header round-trips through
`encode_header` / `decode_header`. Empty and 11-byte spans fail before any
index. Packets with `CC != 0` or `X == 1` are rejected. Do not call
`htons`, open a socket, or add SRTP.
