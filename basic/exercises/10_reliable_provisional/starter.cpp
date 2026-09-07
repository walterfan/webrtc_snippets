#include "starter.hpp"

namespace practice::day10 {

// TODO(day10): Implement parse_rack, requires_prack, and matches_prack for
// the bounded RFC 3262 reliable-provisional rules (RSeq/RAck/PRACK; match
// against separately supplied expected_method). Leave those definitions
// absent until the learner supplies them; Solution::run stays render-only.

int Solution::run(std::ostream &out) {
  return render_todo(
      out, "10", "Match Reliable Provisional Responses",
      "183 + 100rel requires PRACK; RAck \"1 1 INVITE\" matches "
      "rseq/cseq/expected_method");
}

} // namespace practice::day10
