#include "starter.hpp"

namespace practice::day18 {

// TODO(day18): Implement make_report with zero-division and numeric-boundary
// handling.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "18", "Compute RTCP Receiver Statistics",
      "define zero-packet and numeric-boundary behavior before division");
}

} // namespace practice::day18
