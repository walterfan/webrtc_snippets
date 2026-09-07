#include "starter.hpp"

namespace practice::day21 {

// TODO(day21): Implement deterministic fake ICE checks with bounded waits and
// error mapping.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "21", "Run Parallel ICE Checks",
      "model static outcomes with futures; do not create sockets");
}

} // namespace practice::day21
