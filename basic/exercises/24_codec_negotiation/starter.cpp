#include "starter.hpp"

namespace practice::day24 {

// TODO(day24): Implement codec matching, preference, duplicate, and fmtp rules.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "24", "Negotiate Codecs",
      "compare fixed offer/answer data with explicit fmtp rules");
}

} // namespace practice::day24
