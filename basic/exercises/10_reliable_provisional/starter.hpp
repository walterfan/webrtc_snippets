/*
Requirements:
- Parse RAck as exactly rseq, invite CSeq, and method; own the method string.
- Require PRACK only for non-100 provisionals when 100rel is present.
- Match PRACK only when RSeq, INVITE CSeq, and expected_method all agree.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace practice::day10 {

struct RAck {
  unsigned rseq;
  unsigned invite_cseq;
  std::string method;
};

[[nodiscard]] std::optional<RAck>
parse_rack(std::string_view value);

[[nodiscard]] bool requires_prack(unsigned status_code,
                                  bool has_100rel);

[[nodiscard]] bool matches_prack(unsigned rseq,
                                 unsigned invite_cseq,
                                 std::string_view expected_method,
                                 const RAck &rack);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day10
