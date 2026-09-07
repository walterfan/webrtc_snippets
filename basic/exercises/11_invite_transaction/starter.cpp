#include "starter.hpp"

namespace practice::day11 {

// TODO(day11): Implement transition (bounded INVITE client table with
// Accepted) and ack_owner (2xx UA-core vs non-2xx transaction ACK). Leave
// those definitions absent until the learner supplies them; Solution::run
// stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "11", "Model the INVITE Transaction",
      "calling+provisional→proceeding; 2xx→accepted (UA-core ACK); "
      "487→completed then ack→terminated");
}

} // namespace practice::day11
