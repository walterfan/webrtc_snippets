#include "starter.hpp"

namespace practice::day09 {

// TODO(day09): Implement accept_answer for the bounded RFC 3264 session-level
// Offer/Answer rules (media count/order, port-zero rejection, offered
// payloads, Direction). Leave that definition absent until the learner
// supplies it; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "09", "Validate an Offer/Answer",
      "audio + rejected video (port 0); payload subset; no SDP text parse");
}

} // namespace practice::day09
