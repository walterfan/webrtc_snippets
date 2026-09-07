#include "starter.hpp"

namespace practice::day13 {

// TODO(day13): Implement SubscriptionRegistry::subscribe /
// apply_notify and matches_replaces for the bounded RFC 6665 /
// REFER / Replaces profile. Leave those definitions absent until the
// learner supplies them; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "13", "Track Events and Transfer",
      "presence/refer NOTIFY states; exact Replaces Call-ID/tags");
}

} // namespace practice::day13
