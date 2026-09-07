/*
Requirements:
- Parse one Via value beginning with SIP/2.0/<transport>.
- Require non-empty sent-by and exactly one non-empty branch.
- Treat parameter names as case-insensitive; honor empty vs numeric rport.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace practice::day06 {

enum class Transport { udp, tcp, tls, ws, wss };

struct ViaHop {
  Transport transport;
  std::string sent_by;
  std::string branch;
  std::optional<std::string> received;
  std::optional<unsigned short> rport;
  bool rport_requested{};
};

[[nodiscard]] std::optional<ViaHop>
parse_via_hop(std::string_view value);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day06
