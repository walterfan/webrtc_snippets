#include "starter.hpp"

namespace practice::day16 {

// TODO(day16): Implement validated RtpPacketView construction and bounded
// spans.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "16", "Create a Zero-Copy RTP View",
      "return spans that never outlive caller-owned packet storage");
}

} // namespace practice::day16
