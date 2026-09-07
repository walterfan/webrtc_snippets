/*
Requirements:
- Implement `parse_headers` for the stated CRLF-delimited teaching subset.
- Preserve input order and duplicate field names; reject a line that lacks `:`.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <string>
#include <string_view>
#include <vector>
namespace practice::day02 {

struct Header {
  std::string name;
  std::string value;
};

[[nodiscard]] std::vector<Header> parse_headers(std::string_view block);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day02
