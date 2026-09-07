#include "starter.hpp"

namespace practice::day20 {

// TODO(day20): Implement a data-race-free queue with close and stop-token
// wakeups.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "20", "Build a Stoppable Packet Queue",
      "use scoped locking and bounded, deterministic stop tests");
}

} // namespace practice::day20
