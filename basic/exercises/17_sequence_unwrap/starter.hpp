/*
Requirements:
- Define one rollover and bounded reordering policy using fixed-width unsigned
values.
- Avoid signed overflow and keep resulting extended values deterministic.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstdint>
namespace practice::day17 {

class SequenceUnwrapper {
public:
  [[nodiscard]] std::uint64_t unwrap(std::uint16_t sequence);

private:
  static constexpr int kReorderBound = 64;
  bool initialized_{false};
  std::uint16_t last_sequence_{};
  std::uint64_t last_extended_{};
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day17
