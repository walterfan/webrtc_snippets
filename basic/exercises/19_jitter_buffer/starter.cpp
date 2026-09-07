#include "starter.hpp"

namespace practice::day19 {

// TODO(day19): Implement ordered buffering, duplicate rejection, and packet
// ownership.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "19", "Build a Simplified Jitter Buffer",
      "order packets locally and define late/duplicate behavior");
}

} // namespace practice::day19
