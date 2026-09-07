/*
Requirements:
- Identify dialogs by Call-ID and a canonical pair of endpoint tags.
- Treat reverse-direction messages as the same dialog; keep forks separate.
- Ignore pre-dialog requests and incomplete identity fields.
*/
#pragma once
#include "practice/sip_message.hpp"
#include "practice/starter_support.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
namespace practice::day04 {

struct Dialog {
  std::string call_id;
  std::string tag_a;
  std::string tag_b;
  bool early{};
  std::vector<std::size_t> message_indices;
};

[[nodiscard]] std::vector<Dialog>
find_dialogs(const std::vector<practice::SipMessage> &messages);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day04
