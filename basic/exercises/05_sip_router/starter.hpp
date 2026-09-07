/*
Requirements:
- Derive route hops from Record-Route, Route, and Contact for in-dialog
requests.
- Treat ;lr as loose routing; otherwise apply bounded strict-routing hops.
- Ignore requests that lack Call-ID or either dialog tag.
*/
#pragma once
#include "practice/sip_message.hpp"
#include "practice/starter_support.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
namespace practice::day05 {

struct RoutePath {
  std::string call_id;
  std::vector<std::string> hops;
  std::vector<std::size_t> message_indices;
};

[[nodiscard]] std::vector<RoutePath>
find_route_paths(const std::vector<practice::SipMessage> &messages);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day05
