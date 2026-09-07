/*
Requirements:
- Take payload ownership, order packets locally, and reject duplicates.
- Define the late-packet rule and return no packet when the buffer is empty.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>
namespace practice::day19 {

struct Packet {
  std::uint64_t sequence{};
  std::vector<std::byte> payload;
};

class JitterBuffer {
public:
  bool push(Packet packet);
  [[nodiscard]] std::optional<Packet> pop_next();

private:
  std::map<std::uint64_t, std::vector<std::byte>> packets_;
  std::optional<std::uint64_t> last_popped_;
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day19
