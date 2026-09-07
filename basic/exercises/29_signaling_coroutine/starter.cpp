#include "starter.hpp"

namespace practice::day29 {

// TODO(day29): Implement SignalTask ownership and deterministic offer/answer
// sequencing.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "29", "Sequence Signaling with Coroutines",
      "build a deterministic in-process coroutine and propagate exceptions");
}

} // namespace practice::day29
