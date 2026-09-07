# Day 04 tips

## Relevant knowledge

RFC 3261 Section 12 identifies a dialog by Call-ID and the two endpoint tags.
An initial request with no To-tag is **pre-dialog** and is not a dialog yet.
A provisional response (100–199) that carries both tags creates an **early**
dialog. A 2xx response **confirms** that dialog (`early == false`). A forked
INVITE that returns two different To-tags produces two dialog identities.

**Tag canonicalization:** store the two tags in sorted order (`tag_a` <
`tag_b` as raw strings). A reverse-direction BYE that swaps From/To then
hashes to the same pair. Do not keep a distinguished local/remote tag in this
exercise; the sorted pair is the identity.

Ignore a message that lacks Call-ID, From, or To, whose Call-ID is empty after
trim, or whose From/To is missing a non-empty `tag=` parameter. An empty
`tag=` does not count.

## Exercise contract

- **Input:** `const std::vector<practice::SipMessage>&`.
- **Output:** `std::vector<Dialog>` in first-seen identity order.
- **Identity:** trimmed Call-ID plus the sorted tag pair.
- **`early`:** `true` when the group is created by a 1xx response; set
  `false` when a 2xx for that identity is later seen. Requests that join an
  existing dialog do not set `early`.
- **Indices:** append each matching message index once, in encounter order.
- **Pre-dialog / incomplete identity:** skip. The opening INVITE in the main
  fixture (no To-tag), the OPTIONS without Call-ID, and the second INVITE
  (no To-tag) are not dialog members.
- Do not implement route processing, target refresh, or usage authorization.

The existing `Solution::check()` expected dialogs are the source of truth:

- `dialog-a` / `from-a` / `to-a` / confirmed / `[1, 2, 4]`
- `dialog-a` / `from-a` / `to-b` / early / `[3]`
- Edge fixture: zero dialogs

## Solution steps

1. Read Call-ID, From, and To. Trim Call-ID; extract `tag=` from each address.
2. If either tag is missing or empty, skip.
3. Sort the two tag strings into `tag_a`, `tag_b`.
4. If that key exists, append the index and clear `early` on 2xx.
5. Otherwise push a new `Dialog` whose `early` flag is `is_provisional(message)`.

## Edge cases and common mistakes

| Case | Existing behavior |
| --- | --- |
| INVITE without To-tag | ignored (pre-dialog) |
| 180 `to-a` then 200 `to-a` then BYE with swapped tags | one confirmed dialog, indices 1, 2, 4 |
| 180 `to-b` | second early dialog |
| OPTIONS missing Call-ID | ignored |
| Empty `tag=` or blank Call-ID (edge fixture) | ignored; zero dialogs |
| Comparing From/To without sorting tags | reverse BYE becomes a third dialog (wrong) |
| Marking a dialog early from the opening INVITE | INVITE has no To-tag and is skipped |

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R 'Day04|SipDialog'
./build/basic/cpp_rtc_practice --check 04
```

Two dialogs on the main fixture; edge fixture empty. The reverse-direction
BYE must join `from-a`/`to-a`, not open a new identity.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Identify dialogs by Call-ID and a canonical pair of endpoint tags.
- Treat reverse-direction messages as the same dialog; keep forks separate.
- Ignore pre-dialog requests and incomplete identity fields.
*/
#pragma once
#include "practice/sip_message.hpp"
#include "practice/starter_support.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
namespace practice::day04 {

struct Dialog {
  std::string call_id;
  std::string tag_a;
  std::string tag_b;
  bool early{};
  std::vector<std::size_t> message_indices;
};

[[nodiscard]] std::vector<Dialog>
find_dialogs(const std::vector<practice::SipMessage> &messages);

class Solution {
public:
  [[nodiscard]] practice::CheckResult check() const;
  int run(std::ostream &out);
};

} // namespace practice::day04
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <algorithm>
#include <optional>
#include <string_view>
#include <utility>

namespace practice::day04 {
namespace {

constexpr std::string_view kDialogTrace =
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: dialog-a\r\n"
    "CSeq: 20 INVITE\r\n\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-a\r\n"
    "Call-ID: dialog-a\r\n"
    "CSeq: 20 INVITE\r\n\r\n"
    "SIP/2.0 200 OK\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-a\r\n"
    "Call-ID: dialog-a\r\n"
    "CSeq: 20 INVITE\r\n\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-b\r\n"
    "Call-ID: dialog-a\r\n"
    "CSeq: 20 INVITE\r\n\r\n"
    "BYE sip:alice@example.com SIP/2.0\r\n"
    "From: <sip:bob@example.com>;tag=to-a\r\n"
    "To: <sip:alice@example.com>;tag=from-a\r\n"
    "Call-ID: dialog-a\r\n"
    "CSeq: 21 BYE\r\n\r\n"
    "OPTIONS sip:bob@example.com SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-a\r\n"
    "CSeq: 1 OPTIONS\r\n\r\n"
    "INVITE sip:carol@example.com SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-c\r\n"
    "To: <sip:carol@example.com>\r\n"
    "Call-ID: dialog-c\r\n"
    "CSeq: 30 INVITE\r\n\r\n";

constexpr std::string_view kDialogEdgeTrace =
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "From: <sip:alice@example.com>;tag=from-pre\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: dialog-edge\r\n"
    "CSeq: 1 INVITE\r\n\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "From: <sip:alice@example.com>;tag=\r\n"
    "To: <sip:bob@example.com>;tag=to-empty\r\n"
    "Call-ID: dialog-edge\r\n"
    "CSeq: 1 INVITE\r\n\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "From: <sip:alice@example.com>;tag=from-nocallid\r\n"
    "To: <sip:bob@example.com>;tag=to-nocallid\r\n"
    "CSeq: 1 INVITE\r\n\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "From: <sip:alice@example.com>;tag=from-blankid\r\n"
    "To: <sip:bob@example.com>;tag=to-blankid\r\n"
    "Call-ID: \r\n"
    "CSeq: 1 INVITE\r\n\r\n";



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

[[nodiscard]] std::optional<std::size_t>
find_group(const std::vector<Dialog> &dialogs, const DialogKey &key) {
  for (std::size_t i = 0; i < dialogs.size(); ++i) {
    const auto &dialog = dialogs[i];
    if (dialog.call_id == key.call_id && dialog.tag_a == key.tag_a &&
        dialog.tag_b == key.tag_b) {
      return i;
    }
  }
  return std::nullopt;
}

void add_index_once(std::vector<std::size_t> &indices, std::size_t index) {
  if (std::find(indices.begin(), indices.end(), index) == indices.end()) {
    indices.push_back(index);
  }
}

[[nodiscard]] bool is_provisional(const practice::SipMessage &message) {
  return message.kind == practice::SipMessage::Kind::response &&
         message.status_code >= 100U && message.status_code <= 199U;
}

[[nodiscard]] bool is_success(const practice::SipMessage &message) {
  return message.kind == practice::SipMessage::Kind::response &&
         message.status_code >= 200U && message.status_code <= 299U;
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

[[nodiscard]] practice::CheckResult
check_groups(std::string_view fixture, const std::vector<Dialog> &expected,
             std::string_view parse_error) {
  const auto messages = practice::parse_message_series(fixture);
  if (!messages) {
    return {practice::CheckState::failed, std::string(parse_error)};
  }
  const auto dialogs = find_dialogs(*messages);
  if (dialogs.size() != expected.size()) {
    return {practice::CheckState::failed,
            "dialog count " + std::to_string(dialogs.size()) +
                " != " + std::to_string(expected.size())};
  }
  for (std::size_t i = 0; i < expected.size(); ++i) {
    const auto &got = dialogs[i];
    const auto &want = expected[i];
    if (got.call_id != want.call_id || got.tag_a != want.tag_a ||
        got.tag_b != want.tag_b) {
      return {practice::CheckState::failed,
              "dialog " + std::to_string(i) + " identity mismatch"};
    }
    if (got.early != want.early) {
      return {practice::CheckState::failed,
              "dialog " + std::to_string(i) + " early=" +
                  (got.early ? "true" : "false")};
    }
    if (got.message_indices != want.message_indices) {
      return {practice::CheckState::failed,
              "dialog " + std::to_string(i) + " indices " +
                  format_indices(got.message_indices) +
                  " != " + format_indices(want.message_indices)};
    }
  }
  return {practice::CheckState::passed, "Day 04 dialog cases passed"};
}

} // namespace

std::vector<Dialog>
find_dialogs(const std::vector<practice::SipMessage> &messages) {
  std::vector<Dialog> dialogs;
  for (std::size_t index = 0; index < messages.size(); ++index) {
    const auto &message = messages[index];
    const auto key = dialog_key(message);
    if (!key) {
      continue;
    }
    if (const auto found = find_group(dialogs, *key)) {
      auto &dialog = dialogs[*found];
      add_index_once(dialog.message_indices, index);
      if (is_success(message)) {
        dialog.early = false;
      }
      continue;
    }
    dialogs.push_back(Dialog{key->call_id, key->tag_a, key->tag_b,
                             is_provisional(message), {index}});
  }
  return dialogs;
}

practice::CheckResult Solution::check() const {
  const std::vector<Dialog> expected{
      {"dialog-a", "from-a", "to-a", false, {1, 2, 4}},
      {"dialog-a", "from-a", "to-b", true, {3}},
  };
  const auto main = check_groups(kDialogTrace, expected,
                                 "group parse failed for dialog fixture");
  if (main.state != practice::CheckState::passed) {
    return main;
  }
  return check_groups(kDialogEdgeTrace, {},
                      "group parse failed for dialog edge fixture");
}

int Solution::run(std::ostream &out) {
  const auto messages = practice::parse_message_series(kDialogTrace);
  if (!messages) {
    out << "failed to parse dialog fixture\n";
    return 1;
  }
  const auto dialogs = find_dialogs(*messages);
  for (const auto &dialog : dialogs) {
    out << "call-id=" << dialog.call_id << " tags=" << dialog.tag_a << "/"
        << dialog.tag_b << " state=" << (dialog.early ? "early" : "confirmed")
        << " indices=" << format_indices(dialog.message_indices) << '\n';
  }
  return check().state == practice::CheckState::passed ? 0 : 1;
}

} // namespace practice::day04
```
