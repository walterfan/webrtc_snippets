#include "practice/sip_message.hpp"

#include <charconv>
#include <cstddef>

namespace practice {
namespace {


std::string_view trim(std::string_view s) {
  while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
    s.remove_prefix(1);
  }
  while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
    s.remove_suffix(1);
  }
  return s;
}

bool split_space(std::string_view line, std::string_view& a, std::string_view& b,
                 std::string_view& c) {
  const auto first = line.find(' ');
  if (first == std::string_view::npos) {
    return false;
  }
  const auto second = line.find(' ', first + 1);
  if (second == std::string_view::npos) {
    return false;
  }
  if (line.find(' ', second + 1) != std::string_view::npos) {
    return false;
  }
  a = line.substr(0, first);
  b = line.substr(first + 1, second - first - 1);
  c = line.substr(second + 1);
  return !a.empty() && !b.empty() && !c.empty();
}

std::optional<SipMessage> parse_one_message(std::string_view segment) {
  if (segment.empty()) {
    return std::nullopt;
  }

  // Every line break inside a message must be CRLF.
  for (std::size_t i = 0; i < segment.size(); ++i) {
    if (segment[i] == '\r' &&
        (i + 1 >= segment.size() || segment[i + 1] != '\n')) {
      return std::nullopt;
    }
    if (segment[i] == '\n' && (i == 0 || segment[i - 1] != '\r')) {
      return std::nullopt;
    }
  }

  const auto first_crlf = segment.find("\r\n");
  std::string_view start_line;
  std::string_view rest;
  if (first_crlf == std::string_view::npos) {
    start_line = segment;
    rest = {};
  } else {
    start_line = segment.substr(0, first_crlf);
    rest = segment.substr(first_crlf + 2);
  }

  if (start_line.empty()) {
    return std::nullopt;
  }

  SipMessage msg;

  if (start_line.starts_with("SIP/2.0 ")) {
    msg.kind = SipMessage::Kind::response;
    auto after_version = start_line.substr(8);
    if (after_version.size() < 3) {
      return std::nullopt;
    }
    const auto code_view = after_version.substr(0, 3);
    unsigned code = 0;
    const auto* begin = code_view.data();
    const auto* end = begin + code_view.size();
    const auto [ptr, ec] = std::from_chars(begin, end, code);
    if (ec != std::errc{} || ptr != end || code < 100 || code > 699) {
      return std::nullopt;
    }
    // Require a space (or end) after the three-digit code when phrase follows;
    // if longer than 3 chars, next char must be space (reason phrase ignored).
    if (after_version.size() > 3 && after_version[3] != ' ') {
      return std::nullopt;
    }
    msg.status_code = code;
  } else {
    std::string_view method;
    std::string_view uri;
    std::string_view version;
    if (!split_space(start_line, method, uri, version)) {
      return std::nullopt;
    }
    if (version != "SIP/2.0") {
      return std::nullopt;
    }
    msg.kind = SipMessage::Kind::request;
    msg.method = std::string(method);
    msg.request_uri = std::string(uri);
  }

  while (!rest.empty()) {
    const auto line_end = rest.find("\r\n");
    std::string_view line;
    if (line_end == std::string_view::npos) {
      line = rest;
      rest = {};
    } else {
      line = rest.substr(0, line_end);
      rest = rest.substr(line_end + 2);
    }

    // Trailing empty line after last header is fine (no more content).
    if (line.empty()) {
      if (!rest.empty()) {
        return std::nullopt;
      }
      break;
    }

    const auto colon = line.find(':');
    if (colon == std::string_view::npos) {
      return std::nullopt;
    }
    const auto name = trim(line.substr(0, colon));
    if (name.empty()) {
      return std::nullopt;
    }
    const auto value = trim(line.substr(colon + 1));
    msg.headers.push_back(SipHeader{std::string(name), std::string(value)});
  }

  return msg;
}

} // namespace


std::optional<std::string_view> SipHeader::get_address() const noexcept {
  if (value.empty()) {
    return std::nullopt;
  }

  const std::string_view value_view{value};
  const auto address_pos = value_view.find("<");
  if (address_pos == std::string_view::npos) {
    constexpr std::string_view tag_parameter = ";tag=";
    const auto tag_pos = value_view.find(tag_parameter);
    return value_view.substr(0, tag_pos == std::string_view::npos
                                     ? value_view.size()
                                     : tag_pos);
  }
  const auto address_end = value_view.find(">", address_pos + 1);
  if (address_end == std::string_view::npos) {
    return std::nullopt;
  }

  if (address_pos + 1 == address_end) {
    return std::nullopt;
  }
  return std::optional<std::string_view>(
      value_view.substr(address_pos + 1, address_end - address_pos - 1));
}



std::optional<std::vector<SipMessage>>
parse_message_series(std::string_view trace) {
  if (trace.empty()) {
    return std::nullopt;
  }

  std::vector<SipMessage> messages;
  std::size_t pos = 0;
  while (pos < trace.size()) {
    const auto sep = trace.find("\r\n\r\n", pos);
    std::string_view segment;
    if (sep == std::string_view::npos) {
      // The final message may omit its terminating blank CRLF line.
      segment = trace.substr(pos);
      if (segment.empty()) {
        return std::nullopt;
      }
      pos = trace.size();
    } else {
      segment = trace.substr(pos, sep - pos);
      pos = sep + 4;
    }

    if (segment.empty()) {
      // This rejects leading and consecutive separators. A single trailing
      // separator never produces a segment because the loop has ended.
      return std::nullopt;
    }

    auto parsed = parse_one_message(segment);
    if (!parsed) {
      return std::nullopt;
    }
    messages.push_back(std::move(*parsed));

    if (sep == std::string_view::npos) {
      break;
    }
  }

  if (messages.empty()) {
    return std::nullopt;
  }
  return messages;
}

std::optional<std::reference_wrapper<const SipHeader>> SipMessage::find_header(std::string_view name) const noexcept {
  for (auto &header : headers) {
    if (header.name == name) {
      return std::cref(header);
    }
  }
  return std::nullopt;
}

} // namespace practice
