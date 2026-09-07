/*
Requirements:
- parse_content_length: trim spaces/tabs; accept 0..65535 with leading
  zeroes; empty/signed/non-digit/partial => malformed; numeric outside
  0..65535 => out_of_range; both use response_code 400; never leak
  exceptions from the public API.
- diagnose: map ErrorKind to 400 (malformed/out_of_range),
  483/420/401/422/423/481/488/491/503 (401 with 407 as proxy equivalent).
- No sockets; no PAI/UUI/SIPREC executable APIs.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <string>
#include <string_view>
#include <variant>

namespace practice::day14 {

enum class ErrorKind {
  malformed,
  out_of_range,
  too_many_hops,
  unsupported_extension,
  auth_challenge,
  interval_too_small,
  interval_too_brief,
  dialog_not_found,
  media_not_acceptable,
  request_pending,
  server_unavailable
};

struct ProtocolError {
  ErrorKind kind;
  unsigned short response_code;
  std::string detail;
};

using ContentLengthResult =
    std::variant<unsigned short, ProtocolError>;

[[nodiscard]] ContentLengthResult
parse_content_length(std::string_view value);

[[nodiscard]] ProtocolError
diagnose(ErrorKind kind, std::string detail);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day14
