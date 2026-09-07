#include "starter.hpp"

namespace practice::day22 {

// TODO(day22): Implement atomic packet/byte updates and a documented snapshot
// guarantee.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "22", "Snapshot Atomic RTP Statistics",
      "choose and document atomic ordering for a teaching counter pair");
}

} // namespace practice::day22
