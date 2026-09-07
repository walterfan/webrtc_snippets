# Day 09 — Offer/Answer

Implement `accept_answer`. Input is a session-level offer and answer as owned
`SessionDescription` values (nested `MediaLine` lists). Output is the accepted
answer description, or no value when the answer violates the bounded RFC 3264
rules below. Focus: nested value types, sequence validation, algorithms, and
`optional` failure results. Never parse SDP text, choose codecs, send RTP, or
prove that an advertised address is reachable.

## Requirements

### Session-level media correspondence

- The answer must preserve the **number and order** of media lines from the
  offer. Reordering or omitting a line is a failure; do not silently drop or
  insert lines.
- Corresponding lines are paired by index (`offer.media[i]` with
  `answer.media[i]`). Keep `kind` aligned with the offer at that index.
- An answer media line with `port == 0` is a **valid rejection**: the line
  remains present in the accepted result; do not remove it from the vector.
- For a non-rejected answer line (`port != 0`), every payload type in
  `answer.media[i].payload_types` must appear in the corresponding offer line.
  A subset (intersection) is allowed; an unoffered payload type is not.
- `Direction` is limited to the four enum values: `sendrecv`, `sendonly`,
  `recvonly`, and `inactive`. Treat direction as part of the per-line check
  after structural correspondence succeeds.
- On success, return an **owned** `SessionDescription` (the accepted answer),
  not a view or reference into either argument.

### Out of scope

SDP wire-text parsing, `a=` attribute grammars, and detailed codec/fmtp
negotiation belong to Days 23–24. This exercise validates only the bounded
session-layer Offer/Answer model above.

## How to start

1. Read the public contract in `starter.hpp` (`Direction`, `MediaLine`,
   `SessionDescription`, `accept_answer`).
2. Implement `accept_answer` in `starter.cpp`; keep `Solution::run` as a
   render helper until the validator is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day09` after wiring a real check.

## Acceptance fixtures

### Happy path — audio plus rejected video

- **Given** an offer with audio then video, and an answer that keeps both
  lines in order but sets the video `port` to `0`
- **When** `accept_answer` runs
- **Then** the result is present, still has two media lines in the same
  order, and the video line remains with `port == 0`

### Happy path — valid payload intersection

- **Given** an offer audio line with payload types `{0, 8, 96}` and an answer
  audio line with `{0, 8}` (non-zero port)
- **When** `accept_answer` runs
- **Then** the result is present and the accepted audio payloads are the
  answer's subset

### Happy path — direction change within the four-value set

- **Given** matching media counts/order and an answer that changes direction
  (for example offer `sendrecv` answered as `recvonly`) using only the
  `Direction` enum values
- **When** `accept_answer` runs
- **Then** the result is present and carries the answer's direction on that
  line

### Happy path — empty sessions

- **Given** an offer and answer that both have an empty `media` vector
- **When** `accept_answer` runs
- **Then** the result is present with an empty `media` list

### Reject — reordered media

- **Given** an offer ordered audio then video, and an answer ordered video
  then audio (same kinds, wrong order)
- **When** `accept_answer` runs
- **Then** the result is empty (`std::nullopt`)

### Reject — omitted media line

- **Given** an offer with two media lines and an answer with only one
- **When** `accept_answer` runs
- **Then** the result is empty

### Reject — unoffered payload

- **Given** a non-rejected answer line whose `payload_types` includes a type
  absent from the corresponding offer line
- **When** `accept_answer` runs
- **Then** the result is empty
