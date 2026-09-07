#include "starter.hpp"

namespace practice::day02 {

const char *sip =
  "Via: SIP/2.0/UDP a\r\n"
  "Via: SIP/2.0/TCP b\r\n"
  "From: \"Alice\" <sip:alice@atlanta.com>;tag=9fxced76sl\r\n"
  "To: Bob <sip:bob@biloxi.com>\r\n"
  "Call-ID: 2xQU9V7rfyb5uhAwH1s83d;1234567890\r\n"
  "CSeq: 314159 INVITE\r\n"
  "Contact: <sip:alice@atlanta.com>\r\n"
  "Content-Type: application/sdp\r\n"
  "Content-Length: 142\r\n";

// TODO(day02): Implement parse_headers and preserve duplicate header order.
int Solution::run(std::ostream &out) {
  out << "--- day 02 ---\n";
  auto headers = parse_headers(sip);

  out << "Parsed " << headers.size() << " headers\n";
  for (const auto &header : headers) {
    out << "Header: " << header.name << " = " << header.value << "\n";
  }
  if (headers.size() != 9) {
    out << "Expected 9 headers, got " << headers.size() << "\n";
    return 1;
  }
  int ret = is_eq(headers.size(), 9);
  ret += is_eq(headers[0].name, "Via");
  ret += is_eq(headers[0].value, "SIP/2.0/UDP a");
  ret += is_eq(headers[1].name, "Via");
  ret += is_eq(headers[1].value, "SIP/2.0/TCP b");
  ret += is_eq(headers[2].name, "From");
  ret += is_eq(headers[2].value, "\"Alice\" <sip:alice@atlanta.com>;tag=9fxced76sl");
  ret += is_eq(headers[3].name, "To");
  ret += is_eq(headers[3].value, "Bob <sip:bob@biloxi.com>");
  ret += is_eq(headers[4].name, "Call-ID");
  ret += is_eq(headers[4].value, "2xQU9V7rfyb5uhAwH1s83d;1234567890");
  ret += is_eq(headers[5].name, "CSeq");
  ret += is_eq(headers[5].value, "314159 INVITE");
  ret += is_eq(headers[6].name, "Contact");
  ret += is_eq(headers[6].value, "<sip:alice@atlanta.com>");
  ret += is_eq(headers[7].name, "Content-Type");
  ret += is_eq(headers[7].value, "application/sdp");
  ret += is_eq(headers[8].name, "Content-Length");
  ret += is_eq(headers[8].value, "142");
  return ret;
}

namespace {

[[nodiscard]] std::string_view trim(std::string_view text) {
  const auto first = text.find_first_not_of(" \t");
  if (first == std::string_view::npos) {
    return {};
  }
  const auto last = text.find_last_not_of(" \t");
  return text.substr(first, last - first + 1);
}

} // namespace

std::vector<Header> parse_headers(std::string_view block) {
  std::vector<Header> headers;
  std::string_view line = block;
  while (!line.empty()) {
    const std::size_t crlf = line.find("\r\n");
    if (crlf == std::string_view::npos) {
      break;
    }
    const std::string_view header = line.substr(0, crlf);
    const std::size_t colon = header.find(':');
    if (colon == std::string_view::npos) {
      break;
    }
    const std::string_view name = trim(header.substr(0, colon));
    const std::string_view value = trim(header.substr(colon + 1));
    headers.emplace_back(std::string(name), std::string(value));
    line = line.substr(crlf + 2);
  }

  return headers;
}

} // namespace practice::day02
