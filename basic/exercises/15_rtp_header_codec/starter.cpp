#include "starter.hpp"

namespace practice::day15 {

// TODO(day15): Implement checked RTP header encoding and decoding with byte
// bounds validation.
int Solution::run(std::ostream &out) {
  return render_todo(
      out, "15", "Encode and Decode an RTP Header",
      "validate version and length before every indexed byte read");
}

} // namespace practice::day15
