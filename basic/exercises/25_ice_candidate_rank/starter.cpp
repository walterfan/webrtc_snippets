#include "starter.hpp"

namespace practice::day25 {

// TODO(day25): Implement stable priority, component, and transport ranking.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "25", "Rank ICE Candidates",
      "use stable ordering and explicit component/transport tie breakers");
}

} // namespace practice::day25
