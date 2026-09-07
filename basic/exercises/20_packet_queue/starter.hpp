/*
Requirements:
- Use scoped synchronization; do not use manual lock/unlock pairs.
- Ensure a blocked pop finishes after data, close, or stop request without
races.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>
namespace practice::day20 {

class PacketQueue {
public:
  bool push(int packet);
  [[nodiscard]] std::optional<int> pop(std::stop_token stop);
  void close();

private:
  static constexpr std::size_t kCapacity = 8;
  std::queue<int> packets_;
  std::mutex mutex_;
  std::condition_variable_any cv_;
  bool closed_{false};
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day20
