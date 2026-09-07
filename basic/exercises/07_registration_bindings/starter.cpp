#include "starter.hpp"

namespace practice::day07 {

// TODO(day07): Implement RegistrationStore::apply and lookup — create/replace
// on positive expires, remove on expires==0, clear on wildcard_remove, and
// return owned bindings in deterministic order. Leave those definitions absent
// until the learner supplies them; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "07", "Manage Registration Bindings",
      "AOR sip:alice@atlanta.com with desktop/mobile/browser Contacts");
}

} // namespace practice::day07
