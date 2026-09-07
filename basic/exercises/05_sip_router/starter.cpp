#include "starter.hpp"

namespace practice::day05 {

std::vector<RoutePath>
find_route_paths(const std::vector<practice::SipMessage> &messages) {
  // TODO(day05): Derive loose- and strict-routing hops from SIP headers.
  (void)messages;
  return {};
}

int Solution::run(std::ostream &out) {
  // TODO(day05): Print a small route-path example after implementing the
  // Record-Route, Route, and Contact handling above.
  return render_todo(out, "05", "Find SIP Route Paths",
                     "derive deterministic loose and strict route hops");
}

} // namespace practice::day05
