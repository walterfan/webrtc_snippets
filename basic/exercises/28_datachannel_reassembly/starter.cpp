#include "starter.hpp"

namespace practice::day28 {

// TODO(day28): Implement bounded fragment ownership, duplicate rejection, and
// abandon cleanup.
int Solution::run(std::ostream &out) {
  return render_todo(out, "28", "Reassemble DataChannel Fragments",
                     "own fragment bytes and bound incomplete-message state");
}

} // namespace practice::day28
