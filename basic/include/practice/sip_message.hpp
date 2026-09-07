#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace practice {

struct SipHeader {
  std::string name;
  std::string value;

  std::optional<std::string_view> get_address() const noexcept;
};

struct SipMessage {
  enum class Kind { request, response };

  Kind kind{Kind::request};
  std::string method;
  std::string request_uri;
  unsigned status_code{};
  std::vector<SipHeader> headers;

  std::optional<std::reference_wrapper<const SipHeader>>
  find_header(std::string_view name) const noexcept;
};

[[nodiscard]] std::optional<std::vector<SipMessage>>
parse_message_series(std::string_view trace);

} // namespace practice
