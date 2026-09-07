/*
Requirements:
- Validate answer media count/order against the offer; port zero rejects a
  line without removing it.
- Non-rejected answer payload types must be offered; Direction is the
  four-value enum only.
- Return an owned accepted SessionDescription, or nullopt on failure.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <vector>

namespace practice::day09 {

enum class Direction { sendrecv, sendonly, recvonly, inactive };

struct MediaLine {
  std::string kind;
  unsigned short port;
  std::vector<unsigned short> payload_types;
  Direction direction;
};

struct SessionDescription {
  std::vector<MediaLine> media;
};

[[nodiscard]] std::optional<SessionDescription>
accept_answer(const SessionDescription &offer,
              const SessionDescription &answer);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day09
