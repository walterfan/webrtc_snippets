#include "practice/exercise.hpp"
#include "../exercises/01_sip_request_line/starter.hpp"
#include "../exercises/02_sip_headers/starter.hpp"

#include <gtest/gtest.h>

namespace {

void require_completed_exercise(int day) {
  const auto exercise = practice::catalog().find(day);
  ASSERT_TRUE(exercise.has_value()) << "Exercise must be registered";

  const practice::CheckResult result = exercise->get().check();
  if (result.state == practice::CheckState::not_implemented) {
    GTEST_SKIP() << result.detail;
  }
  EXPECT_EQ(result.state, practice::CheckState::passed) << result.detail;
}

} // namespace

TEST(Day01, GivenRequestLine_WhenParsed_ThenFieldsAreExtracted) {
  const auto request_line = practice::day01::parse_request_line(
      "INVITE sip:bob@example.com SIP/2.0");
  ASSERT_TRUE(request_line.has_value());
  EXPECT_EQ(request_line->method, "INVITE");
  EXPECT_EQ(request_line->uri, "sip:bob@example.com");
  EXPECT_EQ(request_line->version, "SIP/2.0");

  EXPECT_FALSE(practice::day01::parse_request_line("").has_value());
  EXPECT_FALSE(practice::day01::parse_request_line("INVITE SIP/2.0").has_value());
  EXPECT_FALSE(practice::day01::parse_request_line("INVITE  SIP/2.0").has_value());
  EXPECT_FALSE(practice::day01::parse_request_line(
                   "INVITE sip:bob@example.com SIP/1.0")
                   .has_value());
  EXPECT_FALSE(practice::day01::parse_request_line(
                   "INVITE sip:bob@example.com SIP/2.0 extra")
                   .has_value());
}
TEST(Day02, GivenDuplicateHeaders_WhenParsed_ThenOrderIsPreserved) {
  constexpr const char *kHeaders =
      "Via: SIP/2.0/UDP a\r\n"
      "Via: SIP/2.0/TCP b\r\n"
      "From: \"Alice\" <sip:alice@atlanta.com>;tag=9fxced76sl\r\n"
      "To: Bob <sip:bob@biloxi.com>\r\n"
      "Call-ID: 2xQU9V7rfyb5uhAwH1s83d;1234567890\r\n"
      "CSeq: 314159 INVITE\r\n"
      "Contact: <sip:alice@atlanta.com>\r\n"
      "Content-Type: application/sdp\r\n"
      "Content-Length: 142\r\n";
  const auto headers = practice::day02::parse_headers(kHeaders);
  ASSERT_EQ(headers.size(), 9U);
  EXPECT_EQ(headers[0].name, "Via");
  EXPECT_EQ(headers[0].value, "SIP/2.0/UDP a");
  EXPECT_EQ(headers[1].name, "Via");
  EXPECT_EQ(headers[1].value, "SIP/2.0/TCP b");
  EXPECT_EQ(headers[8].name, "Content-Length");
  EXPECT_EQ(headers[8].value, "142");

  const auto stopped = practice::day02::parse_headers(
      "Via: SIP/2.0/UDP a\r\nNotAHeader\r\nVia: SIP/2.0/TCP b\r\n");
  ASSERT_EQ(stopped.size(), 1U);
  EXPECT_EQ(stopped[0].name, "Via");
}
TEST(Day03, GivenSipMessages_WhenGrouped_ThenTransactionsAreSeparated) {
  require_completed_exercise(3);
}
TEST(Day04, GivenForkedMessages_WhenGrouped_ThenDialogsAreSeparated) {
  require_completed_exercise(4);
}
TEST(Day05, GivenRouteHeaders_WhenProcessed_ThenPathsAreDerived) {
  require_completed_exercise(5);
}
TEST(Day06, GivenViaHop_WhenParsed_ThenRoutingParametersAreValidated) {
  require_completed_exercise(6);
}
TEST(Day07, GivenRegisterUpdates_WhenApplied_ThenBindingsAreMaintained) {
  require_completed_exercise(7);
}
TEST(Day08, GivenResponseCode_WhenClassified_ThenCapabilitiesAreChecked) {
  require_completed_exercise(8);
}
TEST(Day09, GivenOfferAndAnswer_WhenValidated_ThenMediaRulesAreEnforced) {
  require_completed_exercise(9);
}
TEST(Day10, GivenReliableProvisional_WhenMatched_ThenPrackIsRequired) {
  require_completed_exercise(10);
}
TEST(Day11, GivenInviteEvent_WhenApplied_ThenTransactionStateIsValid) {
  require_completed_exercise(11);
}
TEST(Day12, GivenSessionTimer_WhenNegotiated_ThenRefreshIsBounded) {
  require_completed_exercise(12);
}
TEST(Day13, GivenSubscription_WhenNotified_ThenTransferStateIsTracked) {
  require_completed_exercise(13);
}
TEST(Day14, GivenProtocolField_WhenParsed_ThenTypedErrorIsReturned) {
  require_completed_exercise(14);
}
TEST(Day15, GivenRtpHeader_WhenDecoded_ThenBoundsAreValidated) {
  require_completed_exercise(15);
}
TEST(Day16, GivenPacketBytes_WhenViewed_ThenSpansStayWithinBounds) {
  require_completed_exercise(16);
}
TEST(Day17, GivenSequenceRollover_WhenUnwrapped_ThenExtendedValueIncreases) {
  require_completed_exercise(17);
}
TEST(Day18, GivenReceiverInputs_WhenReported_ThenZeroDivisionIsAvoided) {
  require_completed_exercise(18);
}
TEST(Day19, GivenReorderedPackets_WhenBuffered_ThenNextPacketIsOrdered) {
  require_completed_exercise(19);
}
TEST(Day20, GivenStoppedQueue_WhenPopped_ThenWaiterTerminates) {
  require_completed_exercise(20);
}
TEST(Day21, GivenFakeIceChecks_WhenAwaited_ThenResultsAreBounded) {
  require_completed_exercise(21);
}
TEST(Day22, GivenConcurrentCounters_WhenSnapshotted_ThenValuesAreConsistent) {
  require_completed_exercise(22);
}
TEST(Day23, GivenSdpMediaLines_WhenParsed_ThenSectionsAreSeparated) {
  require_completed_exercise(23);
}
TEST(Day24, GivenCodecLists_WhenNegotiated_ThenCompatibleCodecIsSelected) {
  require_completed_exercise(24);
}
TEST(Day25, GivenCandidateTie_WhenRanked_ThenOrderingIsStable) {
  require_completed_exercise(25);
}
TEST(Day26, GivenFingerprintText_WhenValidated_ThenOnlyAllowedSyntaxPasses) {
  require_completed_exercise(26);
}
TEST(Day27, GivenReplaySequence_WhenAccepted_ThenDuplicateIsRejected) {
  require_completed_exercise(27);
}
TEST(Day28, GivenFragments_WhenReassembled_ThenMessageIsDeliveredOnce) {
  require_completed_exercise(28);
}
TEST(Day29, GivenOfferEvent_WhenAwaited_ThenAnswerFollowsOrErrorPropagates) {
  require_completed_exercise(29);
}
TEST(Day30, GivenStateTransition_WhenObserved_ThenOnlyConformingObserversRun) {
  require_completed_exercise(30);
}
