/*
Requirements:
- Parse only the stated line-oriented `m=`/`a=` subset.
- Reject malformed media lines and do not retain unsupported attributes.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <string>
#include <string_view>
#include <vector>
namespace practice::day23 {
struct MediaSection {
  std::string kind;
  std::vector<std::string> attributes;
};
[[nodiscard]] std::vector<MediaSection>
parse_media_sections(std::string_view sdp);
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day23
