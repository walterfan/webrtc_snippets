/*
Requirements:
- Encode only the documented INVITE client transitions (incl. Accepted).
- Return no value for invalid or terminated-state events.
- ack_owner: 2xx → ua_core; 3xx–6xx → invite_transaction; else none.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>

namespace practice::day11 {

enum class State { calling, proceeding, completed, accepted, terminated };

enum class Event {
  provisional,
  success_2xx,
  failure_3xx_6xx,
  timeout,
  transport_error,
  ack
};

[[nodiscard]] std::optional<State> transition(State state, Event event);

enum class AckOwner { invite_transaction, ua_core };

[[nodiscard]] std::optional<AckOwner> ack_owner(unsigned short status_code);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day11
