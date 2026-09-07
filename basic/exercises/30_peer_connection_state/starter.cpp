#include "starter.hpp"

namespace practice::day30 {

// TODO(day30): Implement valid state transitions and constrained observer
// notification.
int Solution::run(std::ostream &out) {
  return render_todo(out, "30", "Orchestrate PeerConnection State",
                     "compose local signaling/ICE/connection state with "
                     "constrained observers");
}

} // namespace practice::day30
