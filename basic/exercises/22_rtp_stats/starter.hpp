/*
Requirements:
- Use atomics with a documented snapshot consistency guarantee.
- Keep worker loops bounded and use RAII-managed threads.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <atomic>
#include <cstdint>
namespace practice::day22 {
struct StatsSnapshot {
  std::uint64_t packets{};
  std::uint64_t bytes{};
};
class RtpStats {
public:
  void add_packet(std::uint64_t bytes);
  [[nodiscard]] StatsSnapshot snapshot() const;

private:
  std::atomic<std::uint64_t> packets_{0};
  std::atomic<std::uint64_t> bytes_{0};
};
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day22
