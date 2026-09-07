#include "starter.hpp"

namespace practice::day27 {

// TODO(day27): Implement the bounded replay-window policy with unsigned
// sequence arithmetic.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "27", "Track an SRTP Replay Window",
      "define rollover and replay-window behavior with unsigned arithmetic");
}

} // namespace practice::day27
