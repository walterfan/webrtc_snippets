/*
Requirements:
- Provide constexpr status_info for the bounded teaching status set only.
- Classify provisional vs final and keep 401 distinct from 407.
- check_required_extensions accepts only fully supported required tags and
  reports unsupported tags in first-seen order without duplicates.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace practice::day08 {

enum class ResponseClass {
  provisional,
  success,
  redirection,
  client_error,
  server_error,
  global_failure
};

struct StatusInfo {
  unsigned short code;
  std::string_view phrase;
  ResponseClass category;
  bool final;
};

[[nodiscard]] constexpr std::optional<StatusInfo>
status_info(unsigned short code);

struct CapabilityResult {
  bool accepted;
  std::vector<std::string> unsupported;
};

[[nodiscard]] CapabilityResult check_required_extensions(
    const std::vector<std::string> &required,
    const std::vector<std::string> &supported);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day08
