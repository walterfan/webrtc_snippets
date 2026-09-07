/*
Requirements:
- Create views only after validating the minimum header boundary.
- Return const spans into caller-owned storage without extending its lifetime.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstddef>
#include <optional>
#include <span>
namespace practice::day16 {

class RtpPacketView {
public:
  [[nodiscard]] static std::optional<RtpPacketView>
  from(std::span<const std::byte> bytes);
  [[nodiscard]] std::span<const std::byte> header() const;
  [[nodiscard]] std::span<const std::byte> payload() const;

private:
  explicit RtpPacketView(std::span<const std::byte> bytes);
  std::span<const std::byte> bytes_{};
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day16
