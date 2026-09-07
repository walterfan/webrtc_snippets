#include "starter.hpp"

namespace practice::day12 {

// TODO(day12): Implement negotiate_session_timer (Min-SE / invalid /
// refresher default), refresh_due (half-interval fake duration),
// refresh_method_allowed (UPDATE early vs re-INVITE confirmed), and
// is_glare_retry (491). Leave those definitions absent until the learner
// supplies them; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "12", "Negotiate Session Refresh",
      "Session-Expires vs Min-SE; half-interval due; UPDATE/re-INVITE; 491");
}

} // namespace practice::day12
