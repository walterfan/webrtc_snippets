/*
Requirements:
- Avoid division by zero and document the no-RTT case.
- Preserve jitter and compute loss from the documented input fields only.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstdint>
#include <optional>
namespace practice::day18 {

struct ReceiverInputs {
  std::uint32_t expected{};
  std::uint32_t received{};
  double jitter{};
  std::uint32_t lsr{};
  std::uint32_t dlsr{};
};

struct ReceiverReport {
  double loss_fraction{};
  double jitter{};
  std::optional<double> round_trip;
};

[[nodiscard]] ReceiverReport make_report(const ReceiverInputs &input);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day18
