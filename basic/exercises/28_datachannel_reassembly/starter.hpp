/*
Requirements:
- Own fragment bytes and bound the amount of incomplete-message state.
- Deliver a message once, reject duplicates, and release state through
`abandon`.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
namespace practice::day28 {
struct Fragment {
  std::uint32_t message_id{};
  bool final{};
  std::vector<std::byte> bytes;
};
class Reassembler {
public:
  [[nodiscard]] std::optional<std::vector<std::byte>> push(Fragment fragment);
  void abandon(std::uint32_t message_id);
};
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day28
