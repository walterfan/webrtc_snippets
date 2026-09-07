#include "starter.hpp"

namespace practice::day04 {

std::vector<Dialog>
find_dialogs(const std::vector<practice::SipMessage> &messages) {
  // TODO(day04): Identify dialogs by Call-ID and the canonical tag pair.
  (void)messages;
  return {};
}

int Solution::run(std::ostream &out) {
  // TODO(day04): Print a small early/confirmed dialog example after
  // implementing the dialog grouping above.
  return render_todo(out, "04", "Find SIP Dialogs",
                     "group Call-ID and endpoint tags, including forks");
}

} // namespace practice::day04
