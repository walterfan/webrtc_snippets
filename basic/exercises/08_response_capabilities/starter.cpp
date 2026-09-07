#include "starter.hpp"

namespace practice::day08 {

// TODO(day08): Implement status_info (bounded constexpr table) and
// check_required_extensions (Require vs Supported option tags). Leave those
// definitions absent until the learner supplies them; Solution::run stays
// render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "08", "Classify SIP Responses and Capabilities",
      "180 Ringing vs 200 OK; Require 100rel/timer against Supported");
}

} // namespace practice::day08
