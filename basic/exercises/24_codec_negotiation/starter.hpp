/*
Requirements:
- Choose the first preferred compatible codec under the documented
name/rate/fmtp rule.
- Return no value for no match and reject duplicate payload types in either
list.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <optional>
#include <string>
#include <vector>
namespace practice::day24 {
struct Codec {
  int payload_type{};
  std::string name;
  unsigned clock_rate{};
  std::string fmtp;
};
[[nodiscard]] std::optional<Codec> negotiate(const std::vector<Codec> &offer,
                                             const std::vector<Codec> &answer);
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day24
