/*
Requirements:
- Model fixed local outcomes only; do not create sockets or contact STUN/TURN.
- Preserve input order, bound waits by the supplied timeout, and map exceptions
to failure.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <chrono>
#include <future>
#include <string>
#include <vector>
namespace practice::day21 {
struct CandidatePair {
  std::string local, remote;
};
enum class CheckOutcome { succeeded, timed_out, failed };
[[nodiscard]] std::vector<CheckOutcome>
check_pairs(const std::vector<CandidatePair> &pairs,
            std::chrono::milliseconds timeout);
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day21
