#include "starter.hpp"

namespace practice::day26 {

// TODO(day26): Implement bounded fingerprint syntax validation; do not add
// cryptography here.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "26", "Validate a DTLS Fingerprint",
      "validate textual syntax only; never claim certificate authentication");
}

} // namespace practice::day26
