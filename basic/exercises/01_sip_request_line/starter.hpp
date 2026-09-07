/*
Requirements:
- Implement `parse_request_line` without returning views into temporary storage.
- Accept exactly three space-separated fields and require `SIP/2.0` as the
version.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <optional>
#include <string_view>
namespace practice::day01 {

struct RequestLine {
  std::string_view method;
  std::string_view uri;
  std::string_view version;
};

[[nodiscard]] std::optional<RequestLine>
parse_request_line(std::string_view input);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day01
