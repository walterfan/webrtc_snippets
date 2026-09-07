#include "practice/exercise.hpp"

#include "../exercises/01_sip_request_line/starter.hpp"
#include "../exercises/02_sip_headers/starter.hpp"
#include "../exercises/03_sip_transaction/starter.hpp"
#include "../exercises/04_sip_dialog/starter.hpp"
#include "../exercises/05_sip_router/starter.hpp"
#include "../exercises/06_via_routing/starter.hpp"
#include "../exercises/07_registration_bindings/starter.hpp"
#include "../exercises/08_response_capabilities/starter.hpp"
#include "../exercises/09_offer_answer/starter.hpp"
#include "../exercises/10_reliable_provisional/starter.hpp"
#include "../exercises/11_invite_transaction/starter.hpp"
#include "../exercises/12_session_refresh/starter.hpp"
#include "../exercises/13_events_transfer/starter.hpp"
#include "../exercises/14_protocol_errors/starter.hpp"
#include "../exercises/15_rtp_header_codec/starter.hpp"
#include "../exercises/16_rtp_packet_view/starter.hpp"
#include "../exercises/17_sequence_unwrap/starter.hpp"
#include "../exercises/18_rtcp_statistics/starter.hpp"
#include "../exercises/19_jitter_buffer/starter.hpp"
#include "../exercises/20_packet_queue/starter.hpp"
#include "../exercises/21_ice_checks/starter.hpp"
#include "../exercises/22_rtp_stats/starter.hpp"
#include "../exercises/23_sdp_media_sections/starter.hpp"
#include "../exercises/24_codec_negotiation/starter.hpp"
#include "../exercises/25_ice_candidate_rank/starter.hpp"
#include "../exercises/26_dtls_fingerprint/starter.hpp"
#include "../exercises/27_srtp_replay_window/starter.hpp"
#include "../exercises/28_datachannel_reassembly/starter.hpp"
#include "../exercises/29_signaling_coroutine/starter.hpp"
#include "../exercises/30_peer_connection_state/starter.hpp"

#include <algorithm>
#include <ostream>
#include <string>
#include <utility>

namespace practice {

namespace {

[[nodiscard]] CheckResult acceptance_todo(int day) {
  return {CheckState::not_implemented,
          "Add the Given/When/Then cases for Day " + std::to_string(day) +
              " to tests/acceptance_test.cpp."};
}

} // namespace

Catalog::Catalog(std::vector<Exercise> exercises)
    : exercises_(std::move(exercises)) {
  std::sort(exercises_.begin(), exercises_.end(),
            [](const Exercise &left, const Exercise &right) {
              return left.day < right.day;
            });
}

const std::vector<Exercise> &Catalog::exercises() const noexcept {
  return exercises_;
}

std::optional<std::reference_wrapper<const Exercise>>
Catalog::find(int day) const {
  const auto it = std::lower_bound(
      exercises_.begin(), exercises_.end(), day,
      [](const Exercise &exercise, int value) { return exercise.day < value; });
  if (it == exercises_.end() || it->day != day) {
    return std::nullopt;
  }
  return std::cref(*it);
}

std::string Catalog::validate() const {
  if (exercises_.size() != 30U) {
    return "catalog must contain exactly 30 exercises";
  }
  for (std::size_t index = 0; index < exercises_.size(); ++index) {
    const Exercise &exercise = exercises_[index];
    if (exercise.day != static_cast<int>(index + 1U) || exercise.slug.empty() ||
        exercise.title.empty() || !exercise.run || !exercise.check) {
      return "catalog contains an invalid or non-contiguous exercise";
    }
  }
  return {};
}

std::string_view check_state_name(CheckState state) noexcept {
  switch (state) {
  case CheckState::not_implemented:
    return "not_implemented";
  case CheckState::failed:
    return "failed";
  case CheckState::passed:
    return "passed";
  }
  return "unknown";
}

void print_check_result(std::ostream &out, const CheckResult &result) {
  out << "Check: " << check_state_name(result.state) << '\n';
  out << result.detail << '\n';
}

const Catalog &catalog() {
  static const Catalog exercises({
      {1, "sip-request-line", "Parse SIP Request-Line", "starter", "45 min",
       "C++20: string_view, optional, value types",
       "SIP method, Request-URI, version",
       "Split a teaching-sized SIP request line without allocating tokens.",
       [](std::ostream &out) { return day01::Solution{}.run(out); },
       [] { return acceptance_todo(1); }},
      {2, "sip-headers", "Model SIP Header List", "starter", "45 min",
       "C++17: struct, vector, structured bindings",
       "SIP header fields and duplicates",
       "Represent repeated header names while preserving input order.",
       [](std::ostream &out) { return day02::Solution{}.run(out); },
       [] { return acceptance_todo(2); }},
      {3, "sip-transaction", "Find SIP Transactions", "intermediate", "75 min",
       "C++17: value types, algorithms, optionals",
       "RFC 3261 transaction identifiers, retransmissions, ACK/CANCEL",
       "Group a series of SIP messages into transactions without confusing "
       "Call-ID with the transaction key.",
       [](std::ostream &out) { return day03::Solution{}.run(out); },
       [] { return acceptance_todo(3); }},
      {4, "sip-dialog", "Find SIP Dialogs", "intermediate", "75 min",
       "C++17: value types, canonical keys, algorithms",
       "RFC 3261 Call-ID/tags, early dialogs, confirmed dialogs, forking",
       "Identify multiple dialogs in a SIP message series, including forked "
       "early dialogs and reverse-direction messages.",
       [](std::ostream &out) { return day04::Solution{}.run(out); },
       [] { return acceptance_todo(4); }},
      {5, "sip-router", "Find SIP Route Paths", "advanced", "90 min",
       "C++17: owned state, ordered containers, string parsing",
       "RFC 3261 Record-Route, Route, Contact, loose and strict routing",
       "Derive deterministic route paths for several dialogs without opening "
       "sockets or resolving network destinations.",
       [](std::ostream &out) { return day05::Solution{}.run(out); },
       [] { return acceptance_todo(5); }},
      {6, "via-routing", "Via Routing Parameters", "starter", "60 min",
       "C++20: string_view, optional, owned values",
       "RFC 3261 Via parameters, RFC 3581 rport",
       "Parse one Via hop and validate routing parameters without network I/O.",
       [](std::ostream &out) { return day06::Solution{}.run(out); },
       [] { return acceptance_todo(6); }},
      {7, "registration-bindings", "Manage Registration Bindings", "starter",
       "60 min", "C++20: owning containers, lookup/update, sorting",
       "RFC 3261 REGISTER, AOR, Contact expiry",
       "Maintain Contact bindings for an AOR with refresh and wildcard "
       "removal.",
       [](std::ostream &out) { return day07::Solution{}.run(out); },
       [] { return acceptance_todo(7); }},
      {8, "response-capabilities", "Classify SIP Responses and Capabilities",
       "starter", "45 min",
       "C++20: constexpr lookup, array, enum classification",
       "SIP response classes, Supported/Require option tags",
       "Classify teaching status codes and check required extensions.",
       [](std::ostream &out) { return day08::Solution{}.run(out); },
       [] { return acceptance_todo(8); }},
      {9, "offer-answer", "Validate an Offer/Answer", "intermediate", "75 min",
       "C++20: nested value types, algorithms, optional",
       "RFC 3264 Offer/Answer media rules",
       "Validate a session-level answer against an offer without parsing SDP "
       "text.",
       [](std::ostream &out) { return day09::Solution{}.run(out); },
       [] { return acceptance_todo(9); }},
      {10, "reliable-provisional", "Match Reliable Provisional Responses",
       "intermediate", "60 min",
       "C++20: bounded parsing, numeric validation, matching",
       "RFC 3262 100rel, RSeq, RAck, PRACK",
       "Decide when PRACK is required and match RAck identifiers.",
       [](std::ostream &out) { return day10::Solution{}.run(out); },
       [] { return acceptance_todo(10); }},
      {11, "invite-transaction", "Model the INVITE Transaction", "intermediate",
       "75 min", "C++20: scoped enums, switch, invalid transitions",
       "INVITE transaction states, RFC 6026 Accepted, ACK ownership",
       "Apply only the documented INVITE transitions and ACK ownership rules.",
       [](std::ostream &out) { return day11::Solution{}.run(out); },
       [] { return acceptance_todo(11); }},
      {12, "session-refresh", "Negotiate Session Refresh", "intermediate",
       "75 min", "C++20: chrono, variant, optional roles",
       "RFC 4028 Session-Expires, RFC 3311 UPDATE, glare",
       "Negotiate session timers and decide when a refresh is due.",
       [](std::ostream &out) { return day12::Solution{}.run(out); },
       [] { return acceptance_todo(12); }},
      {13, "events-transfer", "Track Events and Transfer", "intermediate",
       "75 min", "C++20: composite keys, owned registry, exact matching",
       "RFC 6665 SUBSCRIBE/NOTIFY, RFC 3515 REFER, RFC 3891 Replaces",
       "Track subscription state and match Replaces without executing a "
       "transfer.",
       [](std::ostream &out) { return day13::Solution{}.run(out); },
       [] { return acceptance_todo(13); }},
      {14, "protocol-errors", "Propagate SIP Protocol Errors", "intermediate",
       "60 min", "C++20: variant, exception containment, numeric validation",
       "Typed parse/validation failures and diagnostic codes",
       "Return typed protocol errors at a SIP parsing boundary.",
       [](std::ostream &out) { return day14::Solution{}.run(out); },
       [] { return acceptance_todo(14); }},
      {15, "rtp-header-codec", "Encode and Decode an RTP Header",
       "intermediate", "75 min",
       "C++17: byte, fixed-width integers, bit operations",
       "RTP V/P/X/CC/M/PT and SSRC",
       "Safely process a bounded teaching RTP header.",
       [](std::ostream &out) { return day15::Solution{}.run(out); },
       [] { return acceptance_todo(15); }},
      {16, "rtp-packet-view", "Create a Zero-Copy RTP View", "intermediate",
       "60 min", "C++20: span, const correctness",
       "RTP header and payload boundaries",
       "Expose packet regions without extending input lifetime.",
       [](std::ostream &out) { return day16::Solution{}.run(out); },
       [] { return acceptance_todo(16); }},
      {17, "sequence-unwrap", "Unwrap RTP Sequence Numbers", "intermediate",
       "60 min", "C++17: integer boundaries, immutable values",
       "RTP 16-bit rollover",
       "Track an extended sequence across a documented rollover policy.",
       [](std::ostream &out) { return day17::Solution{}.run(out); },
       [] { return acceptance_todo(17); }},
      {18, "rtcp-statistics", "Compute RTCP Receiver Statistics",
       "intermediate", "75 min", "C++17: numeric algorithms, accumulate",
       "RTCP loss, jitter, LSR/DLSR",
       "Calculate only the metrics defined by fixed test vectors.",
       [](std::ostream &out) { return day18::Solution{}.run(out); },
       [] { return acceptance_todo(18); }},
      {19, "jitter-buffer", "Build a Simplified Jitter Buffer", "intermediate",
       "75 min", "C++17: ordered containers, move, emplace",
       "Packet reordering and lateness",
       "Buffer a small ordered packet set without real-time I/O.",
       [](std::ostream &out) { return day19::Solution{}.run(out); },
       [] { return acceptance_todo(19); }},
      {20, "packet-queue", "Build a Stoppable Packet Queue", "advanced",
       "90 min", "C++20: jthread, stop_token, condition_variable",
       "Media producer/consumer pipeline",
       "Coordinate a bounded queue with a testable stop path.",
       [](std::ostream &out) { return day20::Solution{}.run(out); },
       [] { return acceptance_todo(20); }},
      {21, "ice-checks", "Run Parallel ICE Checks", "advanced", "75 min",
       "C++17: async, future, timeout", "ICE candidate-pair checks",
       "Model fixed local check results; never open sockets.",
       [](std::ostream &out) { return day21::Solution{}.run(out); },
       [] { return acceptance_todo(21); }},
      {22, "rtp-stats", "Snapshot Atomic RTP Statistics", "advanced", "60 min",
       "C++17: atomics, memory ordering", "RTP packet and byte counters",
       "Produce a consistent teaching snapshot under bounded contention.",
       [](std::ostream &out) { return day22::Solution{}.run(out); },
       [] { return acceptance_todo(22); }},
      {23, "sdp-media-sections", "Parse SDP Media Sections", "advanced",
       "75 min", "C++20: ranges, views, string_view", "SDP m= and a= lines",
       "Parse a deliberately narrow SDP subset.",
       [](std::ostream &out) { return day23::Solution{}.run(out); },
       [] { return acceptance_todo(23); }},
      {24, "codec-negotiation", "Negotiate Codecs", "advanced", "75 min",
       "C++20: ranges, set algorithms, projections",
       "RTP payload types, rtpmap, fmtp",
       "Choose a compatible codec using fixed offers and answers.",
       [](std::ostream &out) { return day24::Solution{}.run(out); },
       [] { return acceptance_todo(24); }},
      {25, "ice-candidate-rank", "Rank ICE Candidates", "advanced", "60 min",
       "C++20: three-way comparison, stable_sort", "ICE priority and component",
       "Order candidates deterministically from static data.",
       [](std::ostream &out) { return day25::Solution{}.run(out); },
       [] { return acceptance_todo(25); }},
      {26, "dtls-fingerprint", "Validate a DTLS Fingerprint", "advanced",
       "60 min", "C++20: ranges, validated types", "DTLS fingerprint syntax",
       "Validate only the textual form, not cryptographic authenticity.",
       [](std::ostream &out) { return day26::Solution{}.run(out); },
       [] { return acceptance_todo(26); }},
      {27, "srtp-replay-window", "Track an SRTP Replay Window", "advanced",
       "90 min", "C++17: bitset, bit operations, unsigned arithmetic",
       "SRTP replay defense",
       "Implement a teaching replay-window policy with fixed vectors.",
       [](std::ostream &out) { return day27::Solution{}.run(out); },
       [] { return acceptance_todo(27); }},
      {28, "datachannel-reassembly", "Reassemble DataChannel Fragments",
       "advanced", "90 min", "C++17: map, unordered_map, variant",
       "DataChannel message fragments",
       "Reassemble in-memory fragments under a stated policy.",
       [](std::ostream &out) { return day28::Solution{}.run(out); },
       [] { return acceptance_todo(28); }},
      {29, "signaling-coroutine", "Sequence Signaling with Coroutines",
       "advanced", "90 min", "C++20: coroutine, awaiter, exception propagation",
       "offer/answer and ICE events",
       "Model a deterministic in-process signaling sequence.",
       [](std::ostream &out) { return day29::Solution{}.run(out); },
       [] { return acceptance_todo(29); }},
      {30, "peer-connection-state", "Orchestrate PeerConnection State",
       "advanced", "90 min", "C++20: concepts, generic observer, RAII",
       "signaling, ICE, and connection state",
       "Compose a small, local PeerConnection state model.",
       [](std::ostream &out) { return day30::Solution{}.run(out); },
       [] { return acceptance_todo(30); }},
  });
  return exercises;
}

} // namespace practice
