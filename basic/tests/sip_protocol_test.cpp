#include "../exercises/03_sip_transaction/starter.hpp"
#include "../exercises/04_sip_dialog/starter.hpp"
#include "../exercises/05_sip_router/starter.hpp"
#include "practice/sip_message.hpp"

#include <gtest/gtest.h>

#include <string_view>
#include <vector>

namespace {

constexpr std::string_view kTrace =
    "INVITE sip:bob@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-a\r\n"
    "From: Alice <sip:alice@example.com>;tag=from-a\r\n"
    "To: Bob <sip:bob@example.com>\r\n"
    "Call-ID: call-a@example.com\r\n"
    "CSeq: 1 INVITE\r\n"
    "\r\n"
    "SIP/2.0 180 Ringing\r\n"
    "Via: SIP/2.0/UDP client.example.com;branch=z9hG4bK-a\r\n"
    "From: Alice <sip:alice@example.com>;tag=from-a\r\n"
    "To: Bob <sip:bob@example.com>;tag=to-a\r\n"
    "Call-ID: call-a@example.com\r\n"
    "CSeq: 1 INVITE\r\n"
    "\r\n";

TEST(SipMessageParser, ParsesRequestResponseAndOwnedHeaders) {
  const auto messages = practice::parse_message_series(kTrace);
  ASSERT_TRUE(messages.has_value());
  ASSERT_EQ(messages->size(), 2U);
  EXPECT_EQ(messages->at(0).kind, practice::SipMessage::Kind::request);
  EXPECT_EQ(messages->at(0).method, "INVITE");
  EXPECT_EQ(messages->at(0).request_uri, "sip:bob@example.com");
  EXPECT_EQ(messages->at(1).status_code, 180U);
  ASSERT_EQ(messages->at(0).headers.size(), 5U);
  EXPECT_EQ(messages->at(0).headers.front().name, "Via");
}

TEST(SipMessageParser, RejectsMalformedStartLineAndHeader) {
  EXPECT_FALSE(practice::parse_message_series(
      "INVITE sip:bob@example.com SIP/1.0\r\n\r\n"));
  EXPECT_FALSE(practice::parse_message_series("Via without a colon\r\n\r\n"));
}

TEST(SipHeader, ExtractsAddressWithOrWithoutAngleBrackets) {
  const practice::SipHeader bracketed{
      "From", "<sip:alice@example.com>;tag=from-a"};
  const practice::SipHeader unbracketed{
      "From", "sip:alice@example.com;tag=from-a"};

  ASSERT_TRUE(bracketed.get_address().has_value());
  ASSERT_TRUE(unbracketed.get_address().has_value());
  EXPECT_EQ(*bracketed.get_address(), "sip:alice@example.com");
  EXPECT_EQ(*unbracketed.get_address(), "sip:alice@example.com");
}

TEST(SipMessageParser, AcceptsFinalMessageWithoutSeparator) {
  constexpr std::string_view trace = "OPTIONS sip:bob@example.com SIP/2.0\r\n"
                                     "CSeq: 1 OPTIONS";
  const auto messages = practice::parse_message_series(trace);
  ASSERT_TRUE(messages.has_value());
  ASSERT_EQ(messages->size(), 1U);
  EXPECT_EQ(messages->at(0).method, "OPTIONS");
  ASSERT_EQ(messages->at(0).headers.size(), 1U);
  EXPECT_EQ(messages->at(0).headers.at(0).value, "1 OPTIONS");

  EXPECT_FALSE(
      practice::parse_message_series("OPTIONS sip:bob@example.com SIP/2.0\r\n"
                                     "CSeq: 1 OPTIONS\r\n\r\n\r\n\r\n"));
}

TEST(SipMessageParser, RejectsBareCarriageReturn) {
  EXPECT_FALSE(
      practice::parse_message_series("OPTIONS sip:bob@example.com SIP/2.0\r\n"
                                     "CSeq: 1 OPTIONS\rX"));
  EXPECT_FALSE(
      practice::parse_message_series("OPTIONS sip:bob@example.com SIP/2.0\r\n"
                                     "CSeq: 1 OPTIONS\r"));
}

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

TEST(SipTransaction, GroupsBranchesAndNon2xxAck) {
  GTEST_SKIP() << "Day 03 starter: implement find_transactions first.";
  const auto messages = practice::parse_message_series(kTransactionTrace);
  ASSERT_TRUE(messages.has_value());
  const auto transactions = practice::day03::find_transactions(*messages);
  ASSERT_EQ(transactions.size(), 3U);
  ASSERT_EQ(transactions.at(0).message_indices,
            (std::vector<std::size_t>{0, 1}));
  ASSERT_EQ(transactions.at(1).message_indices,
            (std::vector<std::size_t>{2, 3, 4}));
  ASSERT_EQ(transactions.at(2).message_indices, (std::vector<std::size_t>{5}));
  EXPECT_EQ(transactions.at(2).method, "CANCEL");
}

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

TEST(SipTransaction, Excludes2xxAckAndIgnoresMalformedKeys) {
  GTEST_SKIP() << "Day 03 starter: implement find_transactions first.";
  const auto messages = practice::parse_message_series(kTransactionEdgeTrace);
  ASSERT_TRUE(messages.has_value());
  ASSERT_EQ(messages->size(), 10U);
  const auto transactions = practice::day03::find_transactions(*messages);
  ASSERT_EQ(transactions.size(), 1U);
  EXPECT_EQ(transactions.at(0).branch, "z9hG4bK-invite-2xx");
  EXPECT_EQ(transactions.at(0).method, "INVITE");
  ASSERT_EQ(transactions.at(0).message_indices,
            (std::vector<std::size_t>{0, 1}));
}

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

TEST(SipDialog, SeparatesForksAndTracksConfirmation) {
  GTEST_SKIP() << "Day 04 starter: implement find_dialogs first.";
  const auto messages = practice::parse_message_series(kDialogTrace);
  ASSERT_TRUE(messages.has_value());
  const auto dialogs = practice::day04::find_dialogs(*messages);
  ASSERT_EQ(dialogs.size(), 2U);
  EXPECT_EQ(dialogs.at(0).call_id, "dialog-a");
  EXPECT_FALSE(dialogs.at(0).early);
  EXPECT_EQ(dialogs.at(0).message_indices, (std::vector<std::size_t>{1, 2, 4}));
  EXPECT_EQ(dialogs.at(1).tag_b, "to-b");
  EXPECT_TRUE(dialogs.at(1).early);
}

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

TEST(SipRouter, DerivesLooseStrictAndDirectPaths) {
  GTEST_SKIP() << "Day 05 starter: implement find_route_paths first.";
  const auto messages = practice::parse_message_series(kRouterTrace);
  ASSERT_TRUE(messages.has_value());
  const auto paths = practice::day05::find_route_paths(*messages);
  ASSERT_EQ(paths.size(), 2U);
  EXPECT_EQ(paths.at(0).call_id, "router-a");
  EXPECT_EQ(paths.at(0).hops,
            (std::vector<std::string>{"sip:proxy-a.example;lr",
                                      "sip:proxy-b.example;lr",
                                      "sip:bob@example.com"}));
  EXPECT_EQ(paths.at(0).message_indices, (std::vector<std::size_t>{0, 1}));
  EXPECT_EQ(
      paths.at(1).hops,
      (std::vector<std::string>{"sip:strict.example", "sip:dave@example.com"}));
}

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

TEST(SipRouter, KeepsStateFromDialogFormingInviteResponses) {
  GTEST_SKIP() << "Day 05 starter: implement find_route_paths first.";
  const auto messages = practice::parse_message_series(kRouterStateTrace);
  ASSERT_TRUE(messages.has_value());
  const auto paths = practice::day05::find_route_paths(*messages);
  ASSERT_EQ(paths.size(), 3U);
  EXPECT_EQ(paths.at(0).hops,
            (std::vector<std::string>{"sip:early.example.com"}));
  EXPECT_EQ(paths.at(0).message_indices, (std::vector<std::size_t>{0, 1}));
  EXPECT_EQ(paths.at(1).hops, (std::vector<std::string>{"sip:good.example;lr",
                                                        "sip:target.example"}));
  EXPECT_EQ(paths.at(1).message_indices, (std::vector<std::size_t>{2, 3}));
  EXPECT_EQ(paths.at(2).hops, (std::vector<std::string>{"sip:good.example;lr",
                                                        "sip:target.example"}));
  EXPECT_EQ(paths.at(2).message_indices, (std::vector<std::size_t>{2, 5}));
}

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

TEST(SipRouter, SkipsMalformedRoutesAndEmptyDestinations) {
  GTEST_SKIP() << "Day 05 starter: implement find_route_paths first.";
  const auto messages = practice::parse_message_series(kMalformedRouteTrace);
  ASSERT_TRUE(messages.has_value());
  const auto paths = practice::day05::find_route_paths(*messages);
  ASSERT_EQ(paths.size(), 1U);
  EXPECT_EQ(paths.at(0).hops, (std::vector<std::string>{"sip:good.example;lr",
                                                        "sip:target.example"}));
  EXPECT_EQ(paths.at(0).message_indices, (std::vector<std::size_t>{0, 3}));

  const std::vector<practice::SipMessage> no_destination{
      {practice::SipMessage::Kind::response,
       {},
       {},
       180,
       {{"From", "<sip:a@example.com>;tag=from-empty"},
        {"To", "<sip:b@example.com>;tag=to-empty"},
        {"Call-ID", "router-empty"},
        {"CSeq", "80 INVITE"}}},
      {practice::SipMessage::Kind::request,
       "BYE",
       {},
       0,
       {{"From", "<sip:a@example.com>;tag=from-empty"},
        {"To", "<sip:b@example.com>;tag=to-empty"},
        {"Call-ID", "router-empty"},
        {"CSeq", "81 BYE"}}},
  };
  EXPECT_TRUE(practice::day05::find_route_paths(no_destination).empty());
}

} // namespace
