# Day 09 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Structure before payloads

Validate that `offer.media.size()` equals `answer.media.size()` first, then
walk indices in lockstep. Confirm `kind` at each index matches before you
look at ports, payload lists, or direction. A length mismatch or reorder is
already a hard failure — do not try to "fix" the answer by sorting or
dropping lines.

## 2. Port zero is a successful rejection

When `answer.media[i].port == 0`, treat that line as a rejected m-line that
**stays in the vector**. You still keep the rejected line in the owned
result; you do not erase it. Payload and direction checks for that index can
be skipped or applied only as your README rules require for rejected lines —
the important teaching point is that rejection is not omission.

## 3. Own the success path

On success, build and return a fresh `SessionDescription` (copy the accepted
answer media). Do not return something that aliases the caller's offer or
answer storage. Prefer value semantics (`std::string`, `std::vector`) already
present on `MediaLine`.

## 4. Self-check before you declare done

Confirm empty offer+answer succeeds with an empty media list. Confirm an
unoffered payload on a non-zero-port line yields `nullopt`. Confirm that a
structurally valid answer still does not prove RTP reachability — this lab
never opens sockets or checks that media will flow.

## Relevant knowledge

RFC 3264 Offer/Answer at the SIP session layer requires the answer to
preserve the **number and order** of the offer's media lines. A line with
`port == 0` is a successful rejection and **stays in the vector**. Codec
intersection is allowed: every payload on a non-rejected answer line must
already appear on the corresponding offer line.

SDP wire-text parsing, `a=` grammars, and detailed codec/fmtp negotiation
belong to Days 23–24 (RFC 8866). This exercise never parses SDP text, never
sends RTP, and never proves that an advertised address is reachable.

C++ technique: compare `offer.media.size()` first, then walk indices in
lockstep. On success return an owned copy of the accepted answer
(`SessionDescription` / `MediaLine` already use value semantics).

## Exercise contract

- **Input:** `const SessionDescription &offer` and `const SessionDescription
  &answer`.
- **Output:** `optional<SessionDescription>` — owned accepted answer, or
  `nullopt`.
- **Count / order:** `answer.media.size()` must equal `offer.media.size()`.
  Pair by index. Reorder or omit is a hard failure.
- **`kind`:** `answer.media[i].kind` must equal `offer.media[i].kind`.
- **`port == 0`:** keep the rejected line; skip payload checks for that
  index.
- **Payloads:** for `port != 0`, every answer payload type must appear in
  the offer line. A subset is allowed.
- **`Direction`:** one of `sendrecv`, `sendonly`, `recvonly`, `inactive`.
  A change within that set is allowed after structural correspondence.
- **Empty sessions:** both `media` vectors empty → success with an empty
  list.

## Solution steps

1. Reject a size mismatch immediately. Do not sort or drop lines.
2. For each index, reject a `kind` mismatch.
3. If `answer.port == 0`, keep the line and continue.
4. Otherwise require every answer payload to be offered.
5. Copy the answer into a fresh `SessionDescription` and return it.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| audio then video, video `port == 0` | success, two lines, video stays |
| offer `{0, 8, 96}`, answer `{0, 8}` | success, accepted subset |
| `sendrecv` answered `recvonly` | success, answer direction kept |
| both media lists empty | success, empty result |
| answer ordered video then audio | `nullopt` |
| offer has two lines, answer has one | `nullopt` |
| non-zero port with unoffered payload | `nullopt` |
| erasing a `port == 0` line | contract violation |

Common mistakes: matching media by `kind` instead of index; treating
rejection as omission; returning a pointer into the caller's answer;
trying to parse SDP text.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Validate answer media count/order against the offer; port zero rejects a
  line without removing it.
- Non-rejected answer payload types must be offered; Direction is the
  four-value enum only.
- Return an owned accepted SessionDescription, or nullopt on failure.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>
#include <vector>

namespace practice::day09 {

enum class Direction { sendrecv, sendonly, recvonly, inactive };

struct MediaLine {
  std::string kind;
  unsigned short port;
  std::vector<unsigned short> payload_types;
  Direction direction;
};

struct SessionDescription {
  std::vector<MediaLine> media;
};

[[nodiscard]] std::optional<SessionDescription>
accept_answer(const SessionDescription &offer,
              const SessionDescription &answer);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day09
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <algorithm>

namespace practice::day09 {
namespace {

[[nodiscard]] bool payload_offered(const std::vector<unsigned short> &offered,
                                   unsigned short payload) {
  return std::find(offered.begin(), offered.end(), payload) != offered.end();
}

} // namespace

std::optional<SessionDescription>
accept_answer(const SessionDescription &offer,
              const SessionDescription &answer) {
  if (offer.media.size() != answer.media.size()) {
    return std::nullopt;
  }
  for (std::size_t i = 0; i < offer.media.size(); ++i) {
    if (offer.media[i].kind != answer.media[i].kind) {
      return std::nullopt;
    }
    if (answer.media[i].port == 0) {
      continue;
    }
    for (const auto payload : answer.media[i].payload_types) {
      if (!payload_offered(offer.media[i].payload_types, payload)) {
        return std::nullopt;
      }
    }
  }
  return answer;
}

int Solution::run(std::ostream &out) {
  out << "--- day 09 ---\n";
  int ret = 0;

  const SessionDescription offer{
      {{"audio", 49170, {0, 8, 96}, Direction::sendrecv},
       {"video", 49172, {96}, Direction::sendrecv}}};
  const SessionDescription rejected{
      {{"audio", 49170, {0, 8}, Direction::recvonly},
       {"video", 0, {96}, Direction::inactive}}};
  const auto accepted = accept_answer(offer, rejected);
  ret += is_ok(accepted.has_value());
  if (accepted) {
    ret += is_eq(accepted->media.size(), 2U);
    ret += is_eq(accepted->media[0].kind, "audio");
    ret += is_eq(accepted->media[1].kind, "video");
    ret += is_eq(accepted->media[1].port, 0);
    ret += is_ok(accepted->media[0].direction == Direction::recvonly);
    ret += is_eq(accepted->media[0].payload_types.size(), 2U);
  }

  const SessionDescription empty_offer;
  const SessionDescription empty_answer;
  const auto empty = accept_answer(empty_offer, empty_answer);
  ret += is_ok(empty.has_value() && empty->media.empty());

  const SessionDescription reordered{
      {{"video", 49172, {96}, Direction::sendrecv},
       {"audio", 49170, {0, 8}, Direction::sendrecv}}};
  ret += is_ok(!accept_answer(offer, reordered).has_value());

  const SessionDescription omitted{
      {{"audio", 49170, {0, 8}, Direction::sendrecv}}};
  ret += is_ok(!accept_answer(offer, omitted).has_value());

  const SessionDescription unoffered{
      {{"audio", 49170, {0, 8, 97}, Direction::sendrecv},
       {"video", 49172, {96}, Direction::sendrecv}}};
  ret += is_ok(!accept_answer(offer, unoffered).has_value());

  out << "accepted lines="
      << (accepted ? accepted->media.size() : 0) << '\n';
  return ret;
}

} // namespace practice::day09
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day09
./build/basic/cpp_rtc_practice --run 09
```

`run()` must return 0. A rejected video line must still be present with
`port == 0`. Reorder, omit, and an unoffered payload must each yield
`nullopt`. Do not parse SDP text or open media sockets.
