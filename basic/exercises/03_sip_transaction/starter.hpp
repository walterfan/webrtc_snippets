/*
Requirements:
- Group messages by top Via branch, sent-by, and CSeq method.
- Attach a non-2xx INVITE ACK; keep CANCEL in its own transaction.
- Ignore messages that lack a usable transaction key.
*/
#pragma once
#include "practice/sip_message.hpp"
#include "practice/starter_support.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
namespace practice::day03 {

struct Transaction {
  std::string branch;
  std::string sent_by;
  std::string method;
  std::vector<std::size_t> message_indices;
};

[[nodiscard]] std::vector<Transaction>
find_transactions(const std::vector<practice::SipMessage> &messages);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day03
