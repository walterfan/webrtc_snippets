/*
Requirements:
- Validate only an allowlisted algorithm and bounded colon-separated hex syntax.
- Reject malformed input and never claim certificate or peer authentication.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <optional>
#include <string>
#include <string_view>
namespace practice::day26 {
struct Fingerprint {
  std::string algorithm;
  std::string hex_digest;
};
[[nodiscard]] std::optional<Fingerprint>
validate_fingerprint(std::string_view input);
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day26
