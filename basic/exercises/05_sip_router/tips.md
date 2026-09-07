# Day 05 tips

## Relevant knowledge

RFC 3261 Sections 12.2.1.1, 12.2.2.1, and 16.12 describe how a UAC builds the
next-hop list from the dialog route set.

**Record-Route / route-set learning (this implementation):** only a
dialog-forming INVITE response (1xx or 2xx whose CSeq method is `INVITE`)
stores Record-Route URIs and Contact. A later 200 OK to BYE is **not**
dialog-forming and must not overwrite the route set. That is why the state
fixture keeps `sip:good.example;lr` after the BYE 200 that advertises
`bad.example`.

**Applying routes to a later request:**

1. Parse every `Route` URI. If any Route/Record-Route/Contact URI is
   malformed (`<>` imbalance, extra `<>`, empty `<>`), skip the whole
   message.
2. If the request has no Route headers and dialog state exists, copy the
   stored Record-Route list.
3. **No hops yet:** use the request-URI; if that is empty, fall back to the
   stored Contact. If both are empty, emit no path.
4. **Loose routing:** the first route URI has an `lr` parameter (with or
   without a value). Hops are the route list plus the request-URI, or
   Contact if the request-URI is empty.
5. **Strict routing:** first hop is the first route; remaining Route URIs
   follow; request-URI is appended when present. Contact is not used as the
   final hop in the strict branch.

Dialog identity uses the same sorted-tag canonicalization as Day 04. Messages
without Call-ID or either tag produce no path.

**Contact-fallback fixture:** a constructed 200 OK plus a BYE with an empty
request-URI. The 200 must carry `CSeq: … INVITE` so
`is_dialog_forming_invite_response` stores Contact. Without that CSeq the
response is ignored, no state exists, the BYE has no destination, and the
path count is `0 != 1`.

No DNS, sockets, or URI grammar beyond extracting the text inside `<>`
(or the trimmed token when `<>` is absent).

## Exercise contract

- **Input:** `const std::vector<practice::SipMessage>&`.
- **Output:** one `RoutePath` per in-dialog request that yields a non-empty
  hop list. `message_indices` is `[response_index, request_index]` when
  dialog-forming state exists, otherwise just the request index.
- **Preserve** Record-Route / Route order.
- **Ignore** requests that lack a dialog identity, that have a malformed
  Route URI, or that resolve to zero hops.
- Existing `Solution::check()` vectors are the source of truth. Do not
  rewrite loose/strict/state/malformed expectations.

## Solution steps

1. Build `dialog_key` (Call-ID + sorted tags). Skip if incomplete.
2. If the message is a 1xx/2xx INVITE response, parse Record-Route and
   optional Contact; store or replace `RouteState` for that key.
3. If the message is not a request, continue.
4. Parse Route URIs (fail the message on a bad URI). Substitute stored
   Record-Routes when the request has none.
5. Build hops using the loose / strict / direct rules above.
6. Attach the stored response index plus the request index.

## Edge cases and common mistakes

| Case | Existing behavior |
| --- | --- |
| Loose Record-Route `proxy-a;lr`, `proxy-b;lr` + BYE request-URI | three hops, URI last |
| Strict `sip:strict.example` + BYE request-URI | `[strict, dave]` |
| Direct Contact, no Record-Route | one hop = request-URI (or Contact if URI empty) |
| Early 180 then BYE with request-URI | path uses the request-URI; indices `[0, 1]` |
| INVITE 200 then later BYE 200 with different Record-Route | later BYE 200 ignored; hops stay on the INVITE 200 |
| Broken `Route: <sip:broken.example` or `Route: <>` | those requests skipped |
| 200 OK contact-fallback **without** CSeq INVITE | no state, `route path count 0 != 1` |
| 200 OK contact-fallback **with** `CSeq: 1 INVITE` | one path `[sip:b@example.com]`, indices `[0, 1]` |
| OPTIONS with only one tag | no path |

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R 'Day05|SipRouter'
./build/basic/cpp_rtc_practice --check 05
```

`check()` must pass. The contact-fallback case must produce exactly one path
after the 200 OK includes `CSeq: 1 INVITE`.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Derive route hops from Record-Route, Route, and Contact for in-dialog requests.
- Treat ;lr as loose routing; otherwise apply bounded strict-routing hops.
- Ignore requests that lack Call-ID or either dialog tag.
*/
#pragma once
#include "practice/sip_message.hpp"
#include "practice/starter_support.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
namespace practice::day05 {

struct RoutePath {
  std::string call_id;
  std::vector<std::string> hops;
  std::vector<std::size_t> message_indices;
};

[[nodiscard]] std::vector<RoutePath>
find_route_paths(const std::vector<practice::SipMessage> &messages);

class Solution {
public:
  [[nodiscard]] practice::CheckResult check() const;
  int run(std::ostream &out);
};

} // namespace practice::day05
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <charconv>
#include <optional>
#include <string_view>
#include <utility>

namespace practice::day05 {
namespace {

constexpr std::string_view kRouterTrace =
    "SIP/2.0 200 OK\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-a\r\n"
    "Call-ID: router-a\r\n"
    "CSeq: 40 INVITE\r\n"
    "Record-Route: <sip:proxy-a.example;lr>\r\n"
    "Record-Route: <sip:proxy-b.example;lr>\r\n"
    "Contact: <sip:bob@example.com>\r\n\r\n"
    "BYE sip:bob@example.com SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-a\r\n"
    "Call-ID: router-a\r\n"
    "CSeq: 41 BYE\r\n"
    "Route: <sip:proxy-a.example;lr>\r\n"
    "Route: <sip:proxy-b.example;lr>\r\n\r\n"
    "SIP/2.0 200 OK\r\n"
    "From: <sip:carol@example.com>;tag=from-b\r\n"
    "To: <sip:dave@example.com>;tag=to-b\r\n"
    "Call-ID: router-b\r\n"
    "CSeq: 50 INVITE\r\n"
    "Record-Route: <sip:strict.example>\r\n"
    "Contact: <sip:dave@example.com>\r\n\r\n"
    "BYE sip:dave@example.com SIP/2.0\r\n"
    "From: <sip:dave@example.com>;tag=to-b\r\n"
    "To: <sip:carol@example.com>;tag=from-b\r\n"
    "Call-ID: router-b\r\n"
    "CSeq: 51 BYE\r\n\r\n"
    "OPTIONS sip:outside.example SIP/2.0\r\n"
    "From: <sip:anonymous@example.com>;tag=outside\r\n"
    "To: <sip:outside.example>\r\n"
    "Call-ID: outside\r\n"
    "CSeq: 1 OPTIONS\r\n\r\n";

constexpr std::string_view kDirectTrace =
    "SIP/2.0 200 OK\r\n"
    "From: <sip:erin@example.com>;tag=from-c\r\n"
    "To: <sip:frank@example.com>;tag=to-c\r\n"
    "Call-ID: router-c\r\n"
    "CSeq: 60 INVITE\r\n"
    "Contact: <sip:frank@example.com>\r\n\r\n"
    "BYE sip:frank@example.com SIP/2.0\r\n"
    "From: <sip:erin@example.com>;tag=from-c\r\n"
    "To: <sip:frank@example.com>;tag=to-c\r\n"
    "Call-ID: router-c\r\n"
    "CSeq: 61 BYE\r\n\r\n"
    "INVITE sip:nobody@example.com SIP/2.0\r\n"
    "From: <sip:x@example.com>;tag=lonely\r\n"
    "To: <sip:nobody@example.com>\r\n"
    "Call-ID: \r\n"
    "CSeq: 1 INVITE\r\n\r\n";

constexpr std::string_view kRouterStateTrace =
    "SIP/2.0 180 Ringing\r\n"
    "From: <sip:alice@example.com>;tag=from-state\r\n"
    "To: <sip:bob@example.com>;tag=to-state\r\n"
    "Call-ID: router-state\r\n"
    "CSeq: 40 INVITE\r\n\r\n"
    "BYE sip:early.example.com SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-state\r\n"
    "To: <sip:bob@example.com>;tag=to-state\r\n"
    "Call-ID: router-state\r\n"
    "CSeq: 41 BYE\r\n\r\n"
    "SIP/2.0 200 OK\r\n"
    "From: <sip:alice@example.com>;tag=from-state\r\n"
    "To: <sip:bob@example.com>;tag=to-state\r\n"
    "Call-ID: router-state\r\n"
    "CSeq: 40 INVITE\r\n"
    "Record-Route: <sip:good.example;lr>\r\n"
    "Contact: <sip:target.example>\r\n\r\n"
    "BYE sip:target.example SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-state\r\n"
    "To: <sip:bob@example.com>;tag=to-state\r\n"
    "Call-ID: router-state\r\n"
    "CSeq: 42 BYE\r\n\r\n"
    "SIP/2.0 200 OK\r\n"
    "From: <sip:alice@example.com>;tag=from-state\r\n"
    "To: <sip:bob@example.com>;tag=to-state\r\n"
    "Call-ID: router-state\r\n"
    "CSeq: 42 BYE\r\n"
    "Record-Route: <sip:bad.example;lr>\r\n"
    "Contact: <sip:bad-target.example>\r\n\r\n"
    "BYE sip:target.example SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-state\r\n"
    "To: <sip:bob@example.com>;tag=to-state\r\n"
    "Call-ID: router-state\r\n"
    "CSeq: 43 BYE\r\n\r\n";

constexpr std::string_view kMalformedRouteTrace =
    "SIP/2.0 200 OK\r\n"
    "From: <sip:alice@example.com>;tag=from-malformed\r\n"
    "To: <sip:bob@example.com>;tag=to-malformed\r\n"
    "Call-ID: router-malformed\r\n"
    "CSeq: 70 INVITE\r\n"
    "Record-Route: <sip:good.example;lr>\r\n"
    "Contact: <sip:target.example>\r\n\r\n"
    "BYE sip:target.example SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-malformed\r\n"
    "To: <sip:bob@example.com>;tag=to-malformed\r\n"
    "Call-ID: router-malformed\r\n"
    "CSeq: 71 BYE\r\n"
    "Route: <sip:broken.example\r\n\r\n"
    "BYE sip:target.example SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-malformed\r\n"
    "To: <sip:bob@example.com>;tag=to-malformed\r\n"
    "Call-ID: router-malformed\r\n"
    "CSeq: 72 BYE\r\n"
    "Route: <>\r\n\r\n"
    "BYE sip:target.example SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-malformed\r\n"
    "To: <sip:bob@example.com>;tag=to-malformed\r\n"
    "Call-ID: router-malformed\r\n"
    "CSeq: 73 BYE\r\n\r\n";


[[nodiscard]] char ascii_lower(char c) {
  if (c >= 'A' && c <= 'Z') {
    return static_cast<char>(c - 'A' + 'a');
  }
  return c;
}

[[nodiscard]] bool ascii_iequals(std::string_view left,
                                 std::string_view right) {
  if (left.size() != right.size()) {
    return false;
  }
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (ascii_lower(left[i]) != ascii_lower(right[i])) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] std::optional<std::string_view>
first_header(const practice::SipMessage &message, std::string_view name) {
  for (const auto &header : message.headers) {
    if (ascii_iequals(header.name, name)) {
      return header.value;
    }
  }
  return std::nullopt;
}

[[nodiscard]] std::vector<std::string_view>
header_values(const practice::SipMessage &message, std::string_view name) {
  std::vector<std::string_view> values;
  for (const auto &header : message.headers) {
    if (ascii_iequals(header.name, name)) {
      values.push_back(header.value);
    }
  }
  return values;
}

[[nodiscard]] std::optional<std::string>
parse_cseq_method(std::string_view value) {
  const auto trimmed = trim(value);
  unsigned number = 0;
  const auto *const begin = trimmed.data();
  const auto *const end = begin + trimmed.size();
  const auto [ptr, ec] = std::from_chars(begin, end, number);
  if (ec != std::errc{} || ptr == begin || ptr == end ||
      (*ptr != ' ' && *ptr != '\t')) {
    return std::nullopt;
  }

  auto rest = std::string_view(ptr, static_cast<std::size_t>(end - ptr));
  while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t')) {
    rest.remove_prefix(1);
  }
  if (rest.empty()) {
    return std::nullopt;
  }
  const auto token_end = rest.find_first_of(" \t");
  const auto method = token_end == std::string_view::npos
                          ? rest
                          : rest.substr(0, token_end);
  if (method.empty()) {
    return std::nullopt;
  }
  if (token_end != std::string_view::npos) {
    rest.remove_prefix(token_end);
    while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t')) {
      rest.remove_prefix(1);
    }
    if (!rest.empty()) {
      return std::nullopt;
    }
  }
  return std::string(method);
}

[[nodiscard]] bool is_dialog_forming_invite_response(
    const practice::SipMessage &message) {
  if (message.kind != practice::SipMessage::Kind::response ||
      !((message.status_code >= 100U && message.status_code <= 199U) ||
        (message.status_code >= 200U && message.status_code <= 299U))) {
    return false;
  }
  const auto cseq = first_header(message, "CSeq");
  return cseq && parse_cseq_method(*cseq) == std::optional<std::string>{"INVITE"};
}

[[nodiscard]] std::optional<std::string> extract_tag(std::string_view value) {
  const auto first_semi = value.find(';');
  if (first_semi == std::string_view::npos) {
    return std::nullopt;
  }
  auto params = value.substr(first_semi + 1);
  while (!params.empty()) {
    const auto next = params.find(';');
    const auto param = trim(params.substr(0, next));
    params = next == std::string_view::npos ? std::string_view{}
                                            : params.substr(next + 1);
    const auto eq = param.find('=');
    if (eq == std::string_view::npos) {
      continue;
    }
    const auto pname = trim(param.substr(0, eq));
    const auto pvalue = trim(param.substr(eq + 1));
    if (ascii_iequals(pname, "tag") && !pvalue.empty()) {
      return std::string(pvalue);
    }
  }
  return std::nullopt;
}

[[nodiscard]] std::optional<std::string>
extract_uri(std::string_view value) {
  const auto open = value.find('<');
  const auto close = value.find('>');
  if (open == std::string_view::npos && close == std::string_view::npos) {
    const auto uri = trim(value);
    if (uri.empty()) {
      return std::nullopt;
    }
    return std::string(uri);
  }
  if (open == std::string_view::npos || close == std::string_view::npos ||
      close <= open || value.find('<', open + 1) != std::string_view::npos ||
      value.find('>', close + 1) != std::string_view::npos) {
    return std::nullopt;
  }
  const auto uri = trim(value.substr(open + 1, close - open - 1));
  if (uri.empty()) {
    return std::nullopt;
  }
  return std::string(uri);
}

[[nodiscard]] std::optional<std::vector<std::string>>
collect_uris(const practice::SipMessage &message, std::string_view name) {
  std::vector<std::string> uris;
  for (const auto value : header_values(message, name)) {
    const auto uri = extract_uri(value);
    if (!uri) {
      return std::nullopt;
    }
    uris.push_back(*uri);
  }
  return uris;
}

[[nodiscard]] bool has_lr_parameter(std::string_view uri) {
  const auto first_semi = uri.find(';');
  if (first_semi == std::string_view::npos) {
    return false;
  }
  auto params = uri.substr(first_semi + 1);
  while (!params.empty()) {
    const auto next = params.find(';');
    const auto param = trim(params.substr(0, next));
    params = next == std::string_view::npos ? std::string_view{}
                                            : params.substr(next + 1);
    const auto eq = param.find('=');
    const auto pname = trim(eq == std::string_view::npos ? param
                                                         : param.substr(0, eq));
    if (ascii_iequals(pname, "lr")) {
      return true;
    }
  }
  return false;
}

struct DialogKey {
  std::string call_id;
  std::string tag_a;
  std::string tag_b;
};

[[nodiscard]] std::optional<DialogKey>
dialog_key(const practice::SipMessage &message) {
  const auto call_id_header = first_header(message, "Call-ID");
  const auto from_header = first_header(message, "From");
  const auto to_header = first_header(message, "To");
  if (!call_id_header || !from_header || !to_header) {
    return std::nullopt;
  }
  const auto call_id = trim(*call_id_header);
  if (call_id.empty()) {
    return std::nullopt;
  }
  const auto from_tag = extract_tag(*from_header);
  const auto to_tag = extract_tag(*to_header);
  if (!from_tag || !to_tag) {
    return std::nullopt;
  }
  std::string tag_a = *from_tag;
  std::string tag_b = *to_tag;
  if (tag_b < tag_a) {
    std::swap(tag_a, tag_b);
  }
  return DialogKey{std::string(call_id), std::move(tag_a), std::move(tag_b)};
}

struct RouteState {
  DialogKey key;
  std::vector<std::string> record_routes;
  std::string contact;
  std::size_t response_index{};
  bool has_response{};
};

[[nodiscard]] std::optional<std::size_t>
find_state(const std::vector<RouteState> &states, const DialogKey &key) {
  for (std::size_t i = 0; i < states.size(); ++i) {
    const auto &state = states[i];
    if (state.key.call_id == key.call_id && state.key.tag_a == key.tag_a &&
        state.key.tag_b == key.tag_b) {
      return i;
    }
  }
  return std::nullopt;
}

[[nodiscard]] std::string
format_indices(const std::vector<std::size_t> &indices) {
  std::string text = "[";
  for (std::size_t i = 0; i < indices.size(); ++i) {
    if (i != 0) {
      text += ", ";
    }
    text += std::to_string(indices[i]);
  }
  text += "]";
  return text;
}

[[nodiscard]] std::string format_hops(const std::vector<std::string> &hops) {
  std::string text = "[";
  for (std::size_t i = 0; i < hops.size(); ++i) {
    if (i != 0) {
      text += ", ";
    }
    text += hops[i];
  }
  text += "]";
  return text;
}

[[nodiscard]] practice::CheckResult
check_paths(const std::vector<practice::SipMessage> &messages,
            const std::vector<RoutePath> &expected) {
  const auto paths = find_route_paths(messages);
  if (paths.size() != expected.size()) {
    return {practice::CheckState::failed,
            "route path count " + std::to_string(paths.size()) +
                " != " + std::to_string(expected.size())};
  }
  for (std::size_t i = 0; i < expected.size(); ++i) {
    const auto &got = paths[i];
    const auto &want = expected[i];
    if (got.call_id != want.call_id) {
      return {practice::CheckState::failed,
              "path " + std::to_string(i) + " call-id " + got.call_id +
                  " != " + want.call_id};
    }
    if (got.hops != want.hops) {
      return {practice::CheckState::failed,
              "path " + std::to_string(i) + " hops " + format_hops(got.hops) +
                  " != " + format_hops(want.hops)};
    }
    if (got.message_indices != want.message_indices) {
      return {practice::CheckState::failed,
              "path " + std::to_string(i) + " indices " +
                  format_indices(got.message_indices) +
                  " != " + format_indices(want.message_indices)};
    }
  }
  return {practice::CheckState::passed, "Day 05 route cases passed"};
}

[[nodiscard]] practice::CheckResult
check_parsed(std::string_view fixture, const std::vector<RoutePath> &expected,
             std::string_view parse_error) {
  const auto messages = practice::parse_message_series(fixture);
  if (!messages) {
    return {practice::CheckState::failed, std::string(parse_error)};
  }
  return check_paths(*messages, expected);
}

} // namespace

std::vector<RoutePath>
find_route_paths(const std::vector<practice::SipMessage> &messages) {
  std::vector<RouteState> states;
  std::vector<RoutePath> paths;
  for (std::size_t index = 0; index < messages.size(); ++index) {
    const auto &message = messages[index];
    const auto key = dialog_key(message);
    if (!key) {
      continue;
    }

    if (is_dialog_forming_invite_response(message)) {
      const auto record_routes = collect_uris(message, "Record-Route");
      if (!record_routes) {
        continue;
      }
      std::string contact;
      if (const auto contact_header = first_header(message, "Contact")) {
        const auto parsed_contact = extract_uri(*contact_header);
        if (!parsed_contact) {
          continue;
        }
        contact = *parsed_contact;
      }
      if (const auto found = find_state(states, *key)) {
        auto &state = states[*found];
        state.record_routes = *record_routes;
        state.contact = std::move(contact);
        state.response_index = index;
        state.has_response = true;
        continue;
      }
      states.push_back(RouteState{*key, *record_routes,
                                  std::move(contact), index, true});
      continue;
    }

    if (message.kind != practice::SipMessage::Kind::request) {
      continue;
    }

    const auto parsed_routes = collect_uris(message, "Route");
    if (!parsed_routes) {
      continue;
    }
    auto routes = *parsed_routes;
    const RouteState *state = nullptr;
    if (const auto found = find_state(states, *key)) {
      state = &states[*found];
    }
    if (routes.empty() && state != nullptr) {
      routes = state->record_routes;
    }

    std::vector<std::string> hops;
    if (routes.empty()) {
      if (!message.request_uri.empty()) {
        hops.push_back(message.request_uri);
      } else if (state != nullptr && !state->contact.empty()) {
        hops.push_back(state->contact);
      }
    } else if (has_lr_parameter(routes.front())) {
      hops = routes;
      const std::string final_uri =
          !message.request_uri.empty()
              ? message.request_uri
              : (state != nullptr ? state->contact : std::string{});
      if (!final_uri.empty()) {
        hops.push_back(final_uri);
      }
    } else {
      hops.push_back(routes.front());
      hops.insert(hops.end(), routes.begin() + 1, routes.end());
      if (!message.request_uri.empty()) {
        hops.push_back(message.request_uri);
      }
    }

    if (hops.empty()) {
      continue;
    }

    std::vector<std::size_t> indices;
    if (state != nullptr && state->has_response) {
      indices.push_back(state->response_index);
    }
    indices.push_back(index);
    paths.push_back(
        RoutePath{key->call_id, std::move(hops), std::move(indices)});
  }
  return paths;
}

practice::CheckResult Solution::check() const {
  const std::vector<RoutePath> expected_main{
      {"router-a",
       {"sip:proxy-a.example;lr", "sip:proxy-b.example;lr",
        "sip:bob@example.com"},
       {0, 1}},
      {"router-b",
       {"sip:strict.example", "sip:dave@example.com"},
       {2, 3}},
  };
  const auto main = check_parsed(kRouterTrace, expected_main,
                                 "group parse failed for router fixture");
  if (main.state != practice::CheckState::passed) {
    return main;
  }

  const std::vector<RoutePath> expected_direct{
      {"router-c", {"sip:frank@example.com"}, {0, 1}},
  };
  const auto direct = check_parsed(
      kDirectTrace, expected_direct,
      "group parse failed for router direct fixture");
  if (direct.state != practice::CheckState::passed) {
    return direct;
  }

  const std::vector<RoutePath> expected_state{
      {"router-state", {"sip:early.example.com"}, {0, 1}},
      {"router-state", {"sip:good.example;lr", "sip:target.example"}, {2, 3}},
      {"router-state", {"sip:good.example;lr", "sip:target.example"}, {2, 5}},
  };
  const auto state = check_parsed(
      kRouterStateTrace, expected_state,
      "group parse failed for router state fixture");
  if (state.state != practice::CheckState::passed) {
    return state;
  }

  const std::vector<RoutePath> expected_malformed{
      {"router-malformed",
       {"sip:good.example;lr", "sip:target.example"},
       {0, 3}},
  };
  const auto malformed = check_parsed(
      kMalformedRouteTrace, expected_malformed,
      "group parse failed for malformed router fixture");
  if (malformed.state != practice::CheckState::passed) {
    return malformed;
  }

  const std::vector<practice::SipMessage> contact_fallback{
      {practice::SipMessage::Kind::response,
       {},
       {},
       200,
       {{"From", "<sip:a@example.com>;tag=from-d"},
        {"To", "<sip:b@example.com>;tag=to-d"},
        {"Call-ID", "router-d"},
        {"CSeq", "1 INVITE"},
        {"Contact", "  <sip:b@example.com>  "}}},
      {practice::SipMessage::Kind::request,
       "BYE",
       {},
       0,
       {{"From", "<sip:a@example.com>;tag=from-d"},
        {"To", "<sip:b@example.com>;tag=to-d"},
        {"Call-ID", "router-d"}}},
  };
  const std::vector<RoutePath> expected_contact{
      {"router-d", {"sip:b@example.com"}, {0, 1}},
  };
  return check_paths(contact_fallback, expected_contact);
}

int Solution::run(std::ostream &out) {
  const auto messages = practice::parse_message_series(kRouterTrace);
  if (!messages) {
    out << "failed to parse router fixture\n";
    return 1;
  }
  const auto paths = find_route_paths(*messages);
  for (const auto &path : paths) {
    out << "call-id=" << path.call_id << " hops=" << format_hops(path.hops)
        << " indices=" << format_indices(path.message_indices) << '\n';
  }
  return check().state == practice::CheckState::passed ? 0 : 1;
}

} // namespace practice::day05
```
