# Day 19 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. Own the payload; order by sequence

`push` takes `Packet` by value so you can `std::move` the payload into a
`std::map<std::uint64_t, std::vector<std::byte>>`. The map’s key order
**is** pop order: `begin()` is the next packet. Do not keep a pointer
into the caller’s vector.

## 2. Duplicates are “already in the map”

If `packets_.contains(sequence)`, return `false` and drop the argument.
Do not overwrite the stored payload. `contains` is C++20.

## 3. Late means `<= last popped`, not “less than the max queued”

`last_popped_` is empty until the first successful `pop_next`. Before
that, sequence `0` is acceptable. After popping `1`, both `1` and `0`
are late even if they were never queued. Late packets are not stored.

## 4. Empty pop does not invent a watermark

`pop_next` on an empty map returns `nullopt` and leaves `last_popped_`
alone. After a real pop, set the watermark to that sequence, then erase.
Self-check: push 3,1,2 → pop 1,2,3; a second `2` while `2` is still
queued fails; after popping `1`, push `1` / `0` fail.

## Relevant knowledge

A jitter buffer absorbs reorder so the decoder sees increasing sequence
numbers. This lab is a tiny ordered store, not RFC 3550 A.8 jitter and
not a timed playout queue. Ownership matters: once `push` returns true,
the buffer holds the bytes.

C++ technique: `map` + `emplace` + `std::move`. `optional<uint64_t>` for
“no pop yet” so sequence `0` is not late at start. No sockets, no sleep.

## Exercise contract

- **Storage:** ordered map of owned payloads; `last_popped_` optional.
- **`push`:** false if late (`sequence <= last_popped_`) or duplicate;
  true after moving the payload in.
- **`pop_next`:** lowest sequence, or `nullopt` if empty. Successful pop
  updates `last_popped_`.
- Move-only payloads; no shared aliasing of caller storage.

## Solution steps

1. Reject late, then reject duplicate, then `emplace` the moved
   payload.
2. `pop_next`: empty → `nullopt`; else take `begin()`, record
   sequence, move payload out, erase.
3. Build `Packet{sequence, payload}` for the return.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| push 3, 1, 2 then three pops | 1, 2, 3 |
| push 2, push 2 again | second is `false` |
| pop 1, then push 1 or 0 | late, `false` |
| pop on empty | `nullopt`, watermark unchanged |
| first push sequence 0 | accepted |

Common mistakes: storing `Packet` copies without moving; using `<=`
against the highest queued sequence instead of last **popped**; setting
`last_popped_ = 0` in the constructor; returning a pointer into the map.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Take payload ownership, order packets locally, and reject duplicates.
- Define the late-packet rule and return no packet when the buffer is empty.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>
namespace practice::day19 {

struct Packet {
  std::uint64_t sequence{};
  std::vector<std::byte> payload;
};

class JitterBuffer {
public:
  bool push(Packet packet);
  [[nodiscard]] std::optional<Packet> pop_next();

private:
  std::map<std::uint64_t, std::vector<std::byte>> packets_;
  std::optional<std::uint64_t> last_popped_;
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day19
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <utility>

namespace practice::day19 {

bool JitterBuffer::push(Packet packet) {
  if (last_popped_.has_value() && packet.sequence <= *last_popped_) {
    return false;
  }
  if (packets_.contains(packet.sequence)) {
    return false;
  }
  const auto sequence = packet.sequence;
  packets_.emplace(sequence, std::move(packet.payload));
  return true;
}

std::optional<Packet> JitterBuffer::pop_next() {
  if (packets_.empty()) {
    return std::nullopt;
  }
  auto first = packets_.begin();
  Packet packet{first->first, std::move(first->second)};
  last_popped_ = packet.sequence;
  packets_.erase(first);
  return packet;
}

int Solution::run(std::ostream &out) {
  out << "--- day 19 ---\n";
  int ret = 0;

  JitterBuffer buffer;
  ret += is_ok(buffer.push({3, {std::byte{0x03}}}));
  ret += is_ok(buffer.push({1, {std::byte{0x01}}}));
  ret += is_ok(buffer.push({2, {std::byte{0x02}}}));
  ret += is_ok(!buffer.push({2, {std::byte{0xFF}}}));

  const auto first = buffer.pop_next();
  ret += is_ok(first.has_value() && first->sequence == 1 &&
               first->payload.size() == 1 &&
               first->payload[0] == std::byte{0x01});
  ret += is_ok(!buffer.push({1, {std::byte{0x01}}}));
  ret += is_ok(!buffer.push({0, {std::byte{0x00}}}));

  const auto second = buffer.pop_next();
  const auto third = buffer.pop_next();
  ret += is_ok(second.has_value() && second->sequence == 2);
  ret += is_ok(third.has_value() && third->sequence == 3);
  ret += is_ok(!buffer.pop_next().has_value());

  JitterBuffer empty_late;
  ret += is_ok(empty_late.push({0, {std::byte{0x00}}}));
  ret += is_ok(empty_late.pop_next().has_value());
  ret += is_ok(!empty_late.push({0, {}}));
  return ret;
}

} // namespace practice::day19
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day19
./build/basic/cpp_rtc_practice --run 19
```

`run()` must return 0. Pushes 3,1,2 pop as 1,2,3. Duplicates and
post-pop late sequences return false. Empty pop is empty. Do not sleep
or open a socket.
