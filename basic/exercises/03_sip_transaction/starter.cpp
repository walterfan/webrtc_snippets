#include "starter.hpp"
#include <algorithm>
#include <charconv>
#include <map>
#include <utility>
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

struct ViaParts {
  std::string branch;
  std::string sent_by;
};
struct CSeqParts {
  unsigned number{};
  std::string method;
};

struct TransactionKey {
  std::string branch;
  std::string sent_by;
  std::string method;

  auto operator<=>(const TransactionKey &) const = default;
};

using TransactionGroups =
    std::map<TransactionKey, std::vector<std::size_t>>;

std::optional<ViaParts> parse_via(std::string_view value) {

  const auto space = value.find(' ');
  if (space == std::string_view::npos) {
    return std::nullopt;
  }

  const auto after_transport = practice::trim(value.substr(space + 1));
  if (after_transport.empty()) {
    return std::nullopt;
  }

  const auto semi = after_transport.find(';');
  const auto sent_by = practice::trim(after_transport.substr(0, semi));
  if (sent_by.empty()) {
    return std::nullopt;
  }

  std::string branch;
  if (semi != std::string_view::npos) {
    auto params = after_transport.substr(semi + 1);
    while (!params.empty()) {
      const auto next = params.find(';');
      const auto param = practice::trim(params.substr(0, next));
      params = next == std::string_view::npos ? std::string_view{}
                                              : params.substr(next + 1);

      const auto equal = param.find('=');
      if (equal == std::string_view::npos) {
        continue;
      }
      const auto name = practice::trim(param.substr(0, equal));
      const auto parameter_value = practice::trim(param.substr(equal + 1));
      if (name == "branch" && !parameter_value.empty()) {
        branch = std::string(parameter_value);
        break;
      }
    }
  }

  if (branch.empty()) {
    return std::nullopt;
  }
  return ViaParts{std::move(branch), std::string(sent_by)};
}

std::optional<CSeqParts> parse_cseq(std::string_view value) {

  const auto trimmed = practice::trim(value);
  unsigned number = 0;
  const auto *const begin = trimmed.data();
  const auto *const end = begin + trimmed.size();
  const auto [ptr, error] = std::from_chars(begin, end, number);
  if (error != std::errc{} || ptr == begin || ptr == end ||
      (*ptr != ' ' && *ptr != '\t')) {
    return std::nullopt;
  }

  auto rest = std::string_view(ptr, static_cast<std::size_t>(end - ptr));
  rest = practice::trim(rest);
  if (rest.empty()) {
    return std::nullopt;
  }

  const auto token_end = rest.find_first_of(" \t");
  const auto method =
      token_end == std::string_view::npos ? rest : rest.substr(0, token_end);
  if (method.empty()) {
    return std::nullopt;
  }
  if (token_end != std::string_view::npos &&
      !practice::trim(rest.substr(token_end)).empty()) {
    return std::nullopt;
  }
  return CSeqParts{number, std::string(method)};
}

std::optional<std::string_view>
find_header_value(const practice::SipMessage &message,
                  std::string_view name) {
  const auto header = message.find_header(name);
  if (!header) {
    return std::nullopt;
  }
  return header->get().value;
}

void add_to_group(TransactionGroups &groups,
                  std::vector<TransactionKey> &first_seen_order,
                  TransactionKey key, std::size_t message_index) {
  const auto [it, inserted] = groups.try_emplace(key);
  if (inserted) {
    first_seen_order.push_back(key);
  }
  it->second.push_back(message_index);
}

bool has_failure_response(
    const std::vector<std::size_t> &message_indices,
    const std::vector<practice::SipMessage> &messages) {
  return std::any_of(
      message_indices.begin(), message_indices.end(),
      [&](const std::size_t index) {
        const auto &message = messages[index];
        return message.kind == practice::SipMessage::Kind::response &&
               message.status_code >= 300U &&
               message.status_code <= 699U;
      });
}

bool has_matching_invite(const std::vector<std::size_t> &message_indices,
                         unsigned cseq_number,
                         const std::vector<practice::SipMessage> &messages) {
  for (const auto index : message_indices) {
    const auto cseq_value = find_header_value(messages[index], "CSeq");
    if (!cseq_value) {
      continue;
    }

    const auto cseq = parse_cseq(*cseq_value);
    if (cseq && cseq->number == cseq_number &&
        cseq->method == "INVITE") {
      return true;
    }
  }
  return false;
}

} // namespace

std::vector<Transaction>
find_transactions(const std::vector<practice::SipMessage> &messages) {
  TransactionGroups groups;
  std::vector<TransactionKey> first_seen_order;

  for (std::size_t index = 0; index < messages.size(); ++index) {
    const auto &message = messages[index];
    const auto via_value = find_header_value(message, "Via");
    const auto cseq_value = find_header_value(message, "CSeq");
    if (!via_value || !cseq_value) {
      continue;
    }

    const auto via = parse_via(*via_value);
    const auto cseq = parse_cseq(*cseq_value);
    if (!via || !cseq) {
      continue;
    }

    const TransactionKey invite_key{via->branch, via->sent_by, "INVITE"};
    if (message.kind == practice::SipMessage::Kind::request &&
        cseq->method == "ACK") {
      const auto invite = groups.find(invite_key);
      if (invite != groups.end() &&
          has_failure_response(invite->second, messages)) {
        invite->second.push_back(index);
      }
      continue;
    }

    if (message.kind == practice::SipMessage::Kind::request &&
        cseq->method == "CANCEL") {
      const auto invite = groups.find(invite_key);
      if (invite == groups.end()) {
        continue;
      }

      if (has_matching_invite(invite->second, cseq->number, messages)) {
        add_to_group(groups, first_seen_order,
                     TransactionKey{via->branch, via->sent_by, cseq->method},
                     index);
      }
      continue;
    }

    add_to_group(groups, first_seen_order,
                 TransactionKey{via->branch, via->sent_by, cseq->method},
                 index);
  }

  std::vector<Transaction> transactions;
  transactions.reserve(first_seen_order.size());
  for (const auto &key : first_seen_order) {
    const auto &indices = groups.at(key);
    transactions.push_back(
        Transaction{key.branch, key.sent_by, key.method, indices});
  }
  return transactions;
}

int Solution::run(std::ostream &out) {
  // TODO(day03): Print a small transaction example after implementing the
  // parser above.

  auto messages = {kTransactionEdgeTrace, kTransactionTrace};

  for (const auto &message : messages) {
    std::optional<std::vector<SipMessage>> parsed =
    parse_message_series(message);
    if (!parsed) {
      return render_todo(out, "03", "Find SIP Transactions",
                        "failed to parse messages");
    }
    std::vector<Transaction> transactions = find_transactions(*parsed);
    out << "\nFound " << transactions.size() << " transactions(branch, sent_by, method)" << std::endl;
    out << "--------------------------------------------------------" << std::endl;
    for (const auto &transaction : transactions) {
      out << transaction.branch << ", " << transaction.sent_by << ", " << transaction.method << std::endl;
    }

    

  }

  
  return 0;
}

} // namespace practice::day03
