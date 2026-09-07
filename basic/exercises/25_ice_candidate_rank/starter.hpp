/*
Requirements:
- Rank static candidates by the documented priority and tie-break rules.
- Preserve input order for equivalent candidates with stable ordering.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <compare>
#include <cstdint>
#include <string>
#include <vector>
namespace practice::day25 {
struct Candidate {
  std::uint32_t priority{};
  unsigned component{};
  std::string transport;
  auto operator<=>(const Candidate &) const = default;
};
[[nodiscard]] std::vector<Candidate>
rank_candidates(std::vector<Candidate> candidates);
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day25
