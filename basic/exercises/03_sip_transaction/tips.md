# Day 03 tips

## Relevant knowledge

RFC 3261 Sections 17.1.3 and 17.2.3 identify a transaction by the top Via
`branch` parameter, the Via sent-by, and the CSeq method. **Call-ID is not
the transaction key.** Two INVITEs that share a Call-ID but use different
branches are two transactions.

ACK and CANCEL ownership (this teaching subset):

- A non-2xx INVITE **ACK** is part of the INVITE transaction. After a 3xx–6xx
  response, an ACK with the same branch and sent-by is appended to that
  INVITE group. The CSeq method of the ACK is `ACK`; matching still uses the
  INVITE group's key.
- A **2xx ACK is not** part of the INVITE transaction. It is ignored here
  rather than opened as its own group.
- **CANCEL** is its own transaction (`method == "CANCEL"`). It is recorded
  only when an INVITE group already exists with the same branch and sent-by
  **and** that INVITE carries the same CSeq number. A CANCEL whose number
  does not match the INVITE (edge fixture: CANCEL 21 vs INVITE 20) is ignored.

Messages without a usable top Via branch/sent-by or a well-formed
`number SP method` CSeq are skipped. Empty `branch=`, a Via with no
parameters, `CSeq: INVITE`, `CSeq: 23INVITE`, extra CSeq tokens, and a
trailing-only CSeq method are all ignored.

The shared helper `practice::parse_message_series` already splits the
header-only trace. This exercise only groups the parsed messages.

## Exercise contract

- **Input:** `const std::vector<practice::SipMessage>&`.
- **Output:** `std::vector<Transaction>` in first-seen group order.
- **Key:** `branch` + `sent_by` + CSeq `method` for ordinary requests and
  responses.
- **ACK:** attach to the INVITE group only when that group already has a
  300–699 response; otherwise skip.
- **CANCEL:** create/append a CANCEL group only when a matching INVITE group
  has the same CSeq number; otherwise skip.
- **Malformed keys:** skip the message; do not fail the whole series.
- Do not implement timers, retransmission I/O, or transport.

The existing `Solution::check()` fixtures are the source of truth. Do not
change their expected indices.

## Solution steps

1. For each message, read the first Via and first CSeq. Parse sent-by (token
   after the transport, before `;`) and a non-empty `branch=` parameter.
   Parse CSeq with `from_chars` plus a single method token.
2. On ACK: look up `(branch, sent-by, "INVITE")`. If that group has a failure
   response, append the ACK index. Always `continue` (never `add_to_group`
   under method `ACK`).
3. On CANCEL: look up the INVITE group and require `has_cseq_number` with
   method `INVITE`. Then `add_to_group` under method `CANCEL`.
4. Otherwise `add_to_group` under the CSeq method (INVITE, responses, …).
5. Preserve first-seen order of groups and append indices in encounter order.

## Edge cases and common mistakes

| Case | Existing behavior |
| --- | --- |
| Two INVITE branches under one Call-ID | two INVITE groups |
| 486 then ACK, same branch as INVITE-b | ACK index joins INVITE-b |
| CANCEL same branch/number as INVITE-a | third group, method CANCEL |
| 200 then ACK (edge fixture) | ACK dropped; only INVITE `[0, 1]` remains |
| Via without `branch` / empty `branch=` | message ignored |
| Bad CSeq forms listed in the edge fixture | message ignored |
| Using Call-ID as the map key | merges the two INVITEs (wrong) |
| Treating every ACK as a new transaction | extra groups (wrong) |

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R 'Day03|SipTransaction'
./build/basic/cpp_rtc_practice --check 03
```

Main fixture groups: INVITE-a `[0, 1]`, INVITE-b `[2, 3, 4]`, CANCEL `[5]`.
Edge fixture: exactly one INVITE group `[0, 1]`.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Group messages by top Via branch, sent-by, and CSeq method.
- Attach a non-2xx INVITE ACK; keep CANCEL in its own transaction.
- Ignore messages that lack a usable transaction key.
*/
#pragma once
#include "practice/sip_message.hpp"
#include "practice/starter_support.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
namespace practice::day03 {

struct Transaction {
  std::string branch;
  std::string sent_by;
  std::string method;
  std::vector<std::size_t> message_indices;
};

[[nodiscard]] std::vector<Transaction>
find_transactions(const std::vector<practice::SipMessage> &messages);

class Solution {
public:
  [[nodiscard]] practice::CheckResult check() const;
  int run(std::ostream &out);
};

} // namespace practice::day03
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <charconv>
#include <optional>
#include <string_view>

namespace practice::day03 {
namespace {

constexpr std::string_view kTransactionTrace =
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-a\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-a\r\n"
    "CSeq: 10 INVITE\r\n\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-a\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>;tag=to-a\r\n"
    "Call-ID: transaction-a\r\n"
    "CSeq: 10 INVITE\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-b\r\n"
    "From: <sip:alice@example.com>;tag=from-b\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-a\r\n"
    "CSeq: 11 INVITE\r\n\r\n"
    "SIP/2.0 486 Busy Here\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-b\r\n"
    "From: <sip:alice@example.com>;tag=from-b\r\n"
    "To: <sip:bob@example.com>;tag=to-b\r\n"
    "Call-ID: transaction-a\r\n"
    "CSeq: 11 INVITE\r\n\r\n"
    "ACK sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-b\r\n"
    "From: <sip:alice@example.com>;tag=from-b\r\n"
    "To: <sip:bob@example.com>;tag=to-b\r\n"
    "Call-ID: transaction-a\r\n"
    "CSeq: 11 ACK\r\n\r\n"
    "CANCEL sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-a\r\n"
    "From: <sip:alice@example.com>;tag=from-a\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-a\r\n"
    "CSeq: 10 CANCEL\r\n\r\n";

constexpr std::string_view kTransactionEdgeTrace =
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-2xx\r\n"
    "From: <sip:alice@example.com>;tag=from-2xx\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 20 INVITE\r\n\r\n"
    "SIP/2.0 200 OK\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-2xx\r\n"
    "From: <sip:alice@example.com>;tag=from-2xx\r\n"
    "To: <sip:bob@example.com>;tag=to-2xx\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 20 INVITE\r\n\r\n"
    "ACK sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-2xx\r\n"
    "From: <sip:alice@example.com>;tag=from-2xx\r\n"
    "To: <sip:bob@example.com>;tag=to-2xx\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 20 ACK\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com\r\n"
    "From: <sip:alice@example.com>;tag=from-nobranch\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 21 INVITE\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=\r\n"
    "From: <sip:alice@example.com>;tag=from-emptybranch\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 22 INVITE\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-bad-cseq\r\n"
    "From: <sip:alice@example.com>;tag=from-badcseq\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: INVITE\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-bad-cseq-no-space\r\n"
    "From: <sip:alice@example.com>;tag=from-bad-cseq-no-space\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 23INVITE\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-bad-cseq-extra\r\n"
    "From: <sip:alice@example.com>;tag=from-bad-cseq-extra\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 24 INVITE extra\r\n\r\n"
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-bad-cseq-missing\r\n"
    "From: <sip:alice@example.com>;tag=from-bad-cseq-missing\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 25 \r\n\r\n"
    "CANCEL sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-invite-2xx\r\n"
    "From: <sip:alice@example.com>;tag=from-2xx\r\n"
    "To: <sip:bob@example.com>\r\n"
    "Call-ID: transaction-edge\r\n"
    "CSeq: 21 CANCEL\r\n\r\n";


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

[[nodiscard]] std::vector<std::string_view>
header_values(const practice::SipMessage &message, std::string_view name) {
  std::vector<std::string_view> values;
  for (const auto &header : message.headers) {
    if (ascii_iequals(header.name, name)) {
      values.emplace_back(header.value);
    }
  }
  return values;
}

struct ViaSentBy {
  std::string branch;
  std::string sent_by;
};

[[nodiscard]] std::optional<ViaSentBy> parse_via(std::string_view value) {
  const auto space = value.find(' ');
  if (space == std::string_view::npos) {
    return std::nullopt;
  }
  const auto after_transport = trim(value.substr(space + 1));
  if (after_transport.empty()) {
    return std::nullopt;
  }
  const auto semi = after_transport.find(';');
  const auto sent_by = trim(after_transport.substr(0, semi));
  if (sent_by.empty()) {
    return std::nullopt;
  }

  std::string branch;
  if (semi != std::string_view::npos) {
    auto params = after_transport.substr(semi + 1);
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
      if (ascii_iequals(pname, "branch") && !pvalue.empty()) {
        branch = std::string(pvalue);
        break;
      }
    }
  }
  if (branch.empty()) {
    return std::nullopt;
  }
  return ViaSentBy{std::move(branch), std::string(sent_by)};
}

struct CSeqParts {
  unsigned number{};
  std::string method;
};

[[nodiscard]] std::optional<CSeqParts> parse_cseq(std::string_view value) {
  const auto trimmed = trim(value);
  unsigned number = 0;
  const auto *const begin = trimmed.data();
  const auto *const end = begin + trimmed.size();
  const auto [ptr, ec] = std::from_chars(begin, end, number);
  if (ec != std::errc{} || ptr == begin || ptr == end ||
      (*ptr != ' ' && *ptr != '\t')) {
    return std::nullopt;
  }

  auto rest =
      std::string_view(ptr, static_cast<std::size_t>(end - ptr));
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
  return CSeqParts{number, std::string(method)};
}

[[nodiscard]] std::optional<std::size_t>
find_group(const std::vector<Transaction> &transactions,
           std::string_view branch, std::string_view sent_by,
           std::string_view method) {
  for (std::size_t i = 0; i < transactions.size(); ++i) {
    const auto &txn = transactions[i];
    if (txn.branch == branch && txn.sent_by == sent_by &&
        txn.method == method) {
      return i;
    }
  }
  return std::nullopt;
}

void add_to_group(std::vector<Transaction> &transactions, std::string branch,
                  std::string sent_by, std::string method, std::size_t index) {
  if (const auto found = find_group(transactions, branch, sent_by, method)) {
    transactions[*found].message_indices.push_back(index);
    return;
  }
  transactions.push_back(
      Transaction{std::move(branch), std::move(sent_by), std::move(method),
                  {index}});
}

[[nodiscard]] bool has_failure_response(
    const Transaction &txn, const std::vector<practice::SipMessage> &messages) {
  for (const std::size_t index : txn.message_indices) {
    if (index >= messages.size()) {
      continue;
    }
    const auto &message = messages[index];
    if (message.kind == practice::SipMessage::Kind::response &&
        message.status_code >= 300U && message.status_code <= 699U) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] bool has_cseq_number(
    const Transaction &txn, unsigned number,
    const std::vector<practice::SipMessage> &messages) {
  for (const std::size_t index : txn.message_indices) {
    if (index >= messages.size()) {
      continue;
    }
    const auto cseqs = header_values(messages[index], "CSeq");
    if (cseqs.empty()) {
      continue;
    }
    const auto cseq = parse_cseq(cseqs.front());
    if (cseq && cseq->number == number && cseq->method == "INVITE") {
      return true;
    }
  }
  return false;
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
check_groups(std::string_view fixture, const std::vector<Transaction> &expected,
             std::string_view parse_error) {
  const auto messages = practice::parse_message_series(fixture);
  if (!messages) {
    return {practice::CheckState::failed, std::string(parse_error)};
  }
  const auto transactions = find_transactions(*messages);
  if (transactions.size() != expected.size()) {
    return {practice::CheckState::failed,
            "group count " + std::to_string(transactions.size()) +
                " != " + std::to_string(expected.size())};
  }
  for (std::size_t i = 0; i < expected.size(); ++i) {
    const auto &got = transactions[i];
    const auto &want = expected[i];
    if (got.message_indices != want.message_indices) {
      return {practice::CheckState::failed,
              "group " + std::to_string(i) + " indices " +
                  format_indices(got.message_indices) +
                  " != " + format_indices(want.message_indices)};
    }
    if (got.branch != want.branch || got.sent_by != want.sent_by ||
        got.method != want.method) {
      return {practice::CheckState::failed,
              "group " + std::to_string(i) + " key mismatch; indices " +
                  format_indices(got.message_indices)};
    }
  }
  return {practice::CheckState::passed, "Day 03 transaction cases passed"};
}

} // namespace

std::vector<Transaction>
find_transactions(const std::vector<practice::SipMessage> &messages) {
  std::vector<Transaction> transactions;
  for (std::size_t index = 0; index < messages.size(); ++index) {
    const auto &message = messages[index];
    const auto vias = header_values(message, "Via");
    if (vias.empty()) {
      continue;
    }
    const auto via = parse_via(vias.front());
    const auto cseqs = header_values(message, "CSeq");
    if (!via || cseqs.empty()) {
      continue;
    }
    const auto cseq = parse_cseq(cseqs.front());
    if (!cseq) {
      continue;
    }

    if (message.kind == practice::SipMessage::Kind::request &&
        cseq->method == "ACK") {
      const auto invite =
          find_group(transactions, via->branch, via->sent_by, "INVITE");
      if (invite && has_failure_response(transactions[*invite], messages)) {
        transactions[*invite].message_indices.push_back(index);
      }
      continue;
    }

    if (message.kind == practice::SipMessage::Kind::request &&
        cseq->method == "CANCEL") {
      const auto invite =
          find_group(transactions, via->branch, via->sent_by, "INVITE");
      if (invite &&
          has_cseq_number(transactions[*invite], cseq->number, messages)) {
        add_to_group(transactions, via->branch, via->sent_by, cseq->method,
                     index);
      }
      continue;
    }

    add_to_group(transactions, via->branch, via->sent_by, cseq->method, index);
  }
  return transactions;
}

practice::CheckResult Solution::check() const {
  const std::vector<Transaction> expected{
      {"z9hG4bK-invite-a", "client.example.com", "INVITE", {0, 1}},
      {"z9hG4bK-invite-b", "client.example.com", "INVITE", {2, 3, 4}},
      {"z9hG4bK-invite-a", "client.example.com", "CANCEL", {5}},
  };
  const auto main = check_groups(kTransactionTrace, expected,
                                 "group parse failed for transaction fixture");
  if (main.state != practice::CheckState::passed) {
    return main;
  }
  const std::vector<Transaction> edge_expected{
      {"z9hG4bK-invite-2xx", "client.example.com", "INVITE", {0, 1}},
  };
  return check_groups(kTransactionEdgeTrace, edge_expected,
                      "group parse failed for edge fixture");
}

int Solution::run(std::ostream &out) {
  const auto messages = practice::parse_message_series(kTransactionTrace);
  if (!messages) {
    out << "failed to parse transaction fixture\n";
    return 1;
  }
  const auto transactions = find_transactions(*messages);
  for (const auto &txn : transactions) {
    out << "branch=" << txn.branch << " sent-by=" << txn.sent_by
        << " method=" << txn.method
        << " indices=" << format_indices(txn.message_indices) << '\n';
  }
  return check().state == practice::CheckState::passed ? 0 : 1;
}

} // namespace practice::day03
```
