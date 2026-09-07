# Day 19 — Simplified Jitter Buffer

Implement `push` and `pop_next`. Example: push sequence 3, 1, 2 and pop
1, 2, 3. Reject duplicate sequence values and reject **late** packets
whose sequence is at or below the last popped sequence. Move payload
ownership into the buffer. Focus: ordered containers, `emplace`, move
semantics.

## Requirements

### Storage

- Own each accepted payload in an ordered container keyed by
  `Packet::sequence` (`std::map<std::uint64_t, std::vector<std::byte>>`).
- Track `last_popped_` as `std::optional<std::uint64_t>`. It is empty
  until the first successful `pop_next`.

### `push`

Take `Packet` by value so the caller can move the payload in.

- **Late:** if `last_popped_` is set and `packet.sequence <=
  *last_popped_`, reject and return `false`. The argument is destroyed
  at the end of the call; do not store it.
- **Duplicate:** if that sequence is already in the map, reject and
  return `false`.
- **Accept:** `emplace` the moved payload and return `true`.

Before any pop, no sequence is late (including `0`). After popping `1`,
both `1` and `0` are late.

### `pop_next`

- If the map is empty, return `std::nullopt` and leave `last_popped_`
  unchanged.
- Otherwise take the **lowest** key (`begin()`), move the payload into
  a `Packet`, set `last_popped_` to that sequence, erase the entry, and
  return the packet.

### Out of scope

RTP timestamp jitter (RFC 3550 A.8), wall-clock pacing, sockets, and
adaptive playout delay are **not** part of this exercise. Sequence
unwrap is Day 17.

## How to start

1. Read the private map and `last_popped_` in `starter.hpp`.
2. Implement `push` / `pop_next` in `starter.cpp`; keep `Solution::run`
   as a render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall.
4. Run `ctest --test-dir build/basic -R Day19` after wiring a real check.

## Acceptance fixtures

### Happy path — reorder

- **Given** pushes of sequences 3, 1, 2
- **When** `pop_next` is called three times
- **Then** output order is 1, 2, 3 and each payload matches the push

### Reject — duplicate

- **Given** an accepted sequence `2` still in the buffer
- **When** `push` of another `2` runs
- **Then** the call returns `false` and the stored payload is unchanged

### Reject — late after pop

- **Given** packets 3, 1, 2 pushed, then one `pop_next` (sequence 1)
- **When** `push` of sequence `1` or `0` runs
- **Then** each is rejected as late

### Empty

- **Given** an empty buffer
- **When** `pop_next` runs
- **Then** the result is empty
