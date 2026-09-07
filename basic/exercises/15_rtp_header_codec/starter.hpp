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
