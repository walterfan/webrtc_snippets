#include "starter.hpp"

namespace practice::day01 {

std::optional<RequestLine> parse_request_line(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }
  const std::size_t space1 = input.find(' ');
  if (space1 == std::string_view::npos || space1 == 0) {
    return std::nullopt;
  }
  const std::size_t space2 = input.find(' ', space1 + 1);
  if (space2 == std::string_view::npos || space2 == space1 + 1) {
    return std::nullopt;
  }
  if (input.find(' ', space2 + 1) != std::string_view::npos) {
    return std::nullopt;
  }
  const std::string_view method = input.substr(0, space1);
  const std::string_view uri = input.substr(space1 + 1, space2 - space1 - 1);
  const std::string_view version = input.substr(space2 + 1);
  if (method.empty() || uri.empty() || version != "SIP/2.0") {
    return std::nullopt;
  }
  return RequestLine{method, uri, version};
}

int Solution::run(std::ostream &out) {
  out << "--- day 01 ---\n";
  auto input = "INVITE sip:bob@example.com SIP/2.0";
  auto request_line = parse_request_line(input);
  if (!request_line) {
    out << "Failed to parse request line\n";
    return 1;
  }
  out << "Input: " << input << "\n";
  out << "Request line: " << request_line->method << " " << request_line->uri
      << " " << request_line->version << "\n";

  int ret = is_ok(request_line.has_value());
  ret += is_ok(request_line->method == "INVITE");
  ret += is_ok(request_line->uri == "sip:bob@example.com");
  ret += is_ok(request_line->version == "SIP/2.0");
  ret += is_ok(!parse_request_line("").has_value());
  ret += is_ok(!parse_request_line("INVITE SIP/2.0").has_value());
  ret += is_ok(!parse_request_line("INVITE  SIP/2.0").has_value());
  ret += is_ok(
      !parse_request_line("INVITE sip:bob@example.com SIP/1.0").has_value());
  ret += is_ok(
      !parse_request_line("INVITE sip:bob@example.com SIP/2.0 extra").has_value());
  return ret;
}

} // namespace practice::day01
