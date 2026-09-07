/*
Requirements:
- Track the documented bounded replay window with unsigned sequence arithmetic.
- Reject duplicates and expired packets; define rollover behavior explicitly.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <bitset>
#include <cstdint>
namespace practice::day27 {
class ReplayWindow {
public:
  [[nodiscard]] bool accept(std::uint16_t sequence);
};
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day27
