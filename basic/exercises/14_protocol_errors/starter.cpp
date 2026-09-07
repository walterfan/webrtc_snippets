#include "starter.hpp"

namespace practice::day14 {

// TODO(day14): Implement parse_content_length and diagnose for the
// typed SIP protocol-error boundary. Leave those definitions absent
// until the learner supplies them; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "14", "Propagate SIP Protocol Errors",
      "malformed vs out_of_range (400); diagnose 483/420/401/…/503");
}

} // namespace practice::day14
