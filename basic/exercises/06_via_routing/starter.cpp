#include "starter.hpp"

namespace practice::day06 {

// TODO(day06): Implement parse_via_hop — transport, sent-by, and Via
// parameters (branch / received / rport). Leave this definition absent until
// the learner supplies it; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "06", "Via Routing Parameters",
      "SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bK-1;"
      "received=192.0.2.1;rport=5070");
}

} // namespace practice::day06
