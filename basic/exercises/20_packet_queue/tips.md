# Day 20 tips

Progressive hints first. Stop once you have a direction. The complete
reference implementation is at the end for comparison after you have
attempted the exercise.

## 1. RAII locks only

`push` and `close` use `std::lock_guard`. `pop` uses `std::unique_lock`
because `wait` unlocks while sleeping. Never call `mutex.unlock()` by
hand. The mutex guards `packets_` and `closed_` together.

## 2. `condition_variable_any` plus the stop-token wait

Store `std::condition_variable_any`, not `std::condition_variable`.
Call `cv_.wait(lock, stop, predicate)` with
`predicate = !packets_.empty() || closed_`. Stop is the overload’s
`stop_token` argument. After `wait` returns, if the queue has an item,
return it (data wins). Only if it is empty do you return `nullopt`.

## 3. Do not use a notify-only `stop_callback`

A callback that only `notify_all`s, paired with
`condition_variable::wait(lock, pred)`, can lose a wakeup between
evaluating the predicate and entering the wait. The C++20
`wait(lock, stop_token, pred)` overload registers stop against that
wait. `close()` still `notify_all`s because close is not a stop token.

## 4. Capacity 8; push never blocks

If `closed_` or `packets_.size() == 8`, `push` returns `false`
immediately. That keeps overflow tests deadlock-free. A successful push
`notify_one`s. Prefer same-thread checks: already-stopped empty pop,
already-stopped pop with data, close then pop, ninth push fails.

## Relevant knowledge

Media threads need a bounded queue that can shut down without leaking a
blocked consumer. C++20 `std::stop_token` / `std::jthread` are the
cooperative cancel path. `std::condition_variable_any` is the CV that
accepts a `stop_token` on `wait`.

Capacity is a fixed teaching bound of **8**. `push` is non-blocking so
a full queue is a boolean, not a second wait that could deadlock a
single-threaded test.

C++ technique: one mutex, one `condition_variable_any`,
`wait(lock, stop, pred)`, RAII locks. No `stop_callback`, no
`sleep_for`, no timed `wait_for`, no sockets.

## Exercise contract

- **State:** queue, mutex, `condition_variable_any`, `closed_`,
  capacity 8.
- **`push`:** false if closed or full; else enqueue, notify one, true.
- **`pop`:** `wait(lock, stop, !empty || closed)`; return front or
  `nullopt`. Data wins over close/stop when an item is present.
- **`close`:** set `closed_`, notify all; remaining items still drain.
- Stop does not set `closed_`.
- Already-requested stop before `pop`: empty → `nullopt`; item present
  → that item.
- No manual lock/unlock. No wall-clock wait. No `stop_callback`.

## Solution steps

1. Implement `close` (flag + `notify_all`).
2. Implement `push` (reject closed/full, else enqueue + `notify_one`).
3. In `pop`, `unique_lock` then `cv_.wait(lock, stop, pred)` with
   `pred = !empty || closed`. Drain one item or return empty.

## Edge cases and common mistakes

| Case | Result |
| --- | --- |
| push 1, pop unstopped | `1`, no block |
| blocked pop + push 42 | `42` |
| `close()` then pop on empty | `nullopt` |
| `request_stop()` **then** pop on empty | `nullopt` (same thread) |
| item queued, token already stopped | return the item |
| close with items still queued | later pop drains them |
| eight pushes then a ninth | ninth is `false` |

Common mistakes: `std::condition_variable` plus a notify-only
`stop_callback`; putting `stop_requested()` only in a homemade
predicate; `push` waiting on full; `notify_one` in `close` (use
`notify_all`); unlocking by hand.

## Complete reference implementation

Copy these files over the exercise `starter.hpp` and `starter.cpp` when you
want a known-good answer. The fenced blocks are the complete C++20 sources.

### `starter.hpp`

```cpp
/*
Requirements:
- Use scoped synchronization; do not use manual lock/unlock pairs.
- Ensure a blocked pop finishes after data, close, or stop request without
races.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>
namespace practice::day20 {

class PacketQueue {
public:
  bool push(int packet);
  [[nodiscard]] std::optional<int> pop(std::stop_token stop);
  void close();

private:
  static constexpr std::size_t kCapacity = 8;
  std::queue<int> packets_;
  std::mutex mutex_;
  std::condition_variable_any cv_;
  bool closed_{false};
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day20
```

### `starter.cpp`

```cpp
#include "starter.hpp"

#include <thread>

namespace practice::day20 {

bool PacketQueue::push(int packet) {
  std::lock_guard lock{mutex_};
  if (closed_ || packets_.size() >= kCapacity) {
    return false;
  }
  packets_.push(packet);
  cv_.notify_one();
  return true;
}

std::optional<int> PacketQueue::pop(std::stop_token stop) {
  std::unique_lock lock{mutex_};
  cv_.wait(lock, stop, [&] { return !packets_.empty() || closed_; });
  if (packets_.empty()) {
    return std::nullopt;
  }
  const int packet = packets_.front();
  packets_.pop();
  return packet;
}

void PacketQueue::close() {
  std::lock_guard lock{mutex_};
  closed_ = true;
  cv_.notify_all();
}

int Solution::run(std::ostream &out) {
  out << "--- day 20 ---\n";
  int ret = 0;

  {
    PacketQueue queue;
    std::stop_source source;
    ret += is_ok(queue.push(1));
    const auto value = queue.pop(source.get_token());
    ret += is_ok(value.has_value() && *value == 1);
  }

  {
    PacketQueue queue;
    queue.close();
    std::stop_source source;
    ret += is_ok(!queue.pop(source.get_token()).has_value());
    ret += is_ok(!queue.push(7));
  }

  {
    PacketQueue queue;
    std::stop_source source;
    source.request_stop();
    ret += is_ok(!queue.pop(source.get_token()).has_value());
    ret += is_ok(queue.push(3));
    ret += is_ok(queue.pop(source.get_token()) == 3);
  }

  {
    PacketQueue queue;
    ret += is_ok(queue.push(9));
    queue.close();
    std::stop_source source;
    ret += is_ok(queue.pop(source.get_token()) == 9);
    ret += is_ok(!queue.pop(source.get_token()).has_value());
    ret += is_ok(!queue.push(8));
  }

  {
    PacketQueue queue;
    std::optional<int> got;
    std::jthread consumer{[&](std::stop_token token) {
      got = queue.pop(token);
    }};
    ret += is_ok(queue.push(42));
    consumer.join();
    ret += is_ok(got.has_value() && *got == 42);
  }

  {
    PacketQueue queue;
    for (int i = 0; i < 8; ++i) {
      ret += is_ok(queue.push(i));
    }
    ret += is_ok(!queue.push(99));
    std::stop_source source;
    ret += is_ok(queue.pop(source.get_token()) == 0);
    ret += is_ok(queue.push(8));
  }
  return ret;
}

} // namespace practice::day20
```

## Self-check

```bash
cmake --build build/basic --target practice_core practice_acceptance_test
ctest --test-dir build/basic --output-on-failure -R Day20
./build/basic/cpp_rtc_practice --run 20
```

`run()` must return 0. Same-thread: `close()` then empty `pop` is
empty; `request_stop()` **before** `pop` on an empty queue is empty;
an already-stopped token still returns a queued item; `close()` drains
queued items. A later push can still wake a waiter. The ninth push
fails. Use `wait(lock, stop, pred)` and RAII locks; do not use a
notify-only `stop_callback`, sleep, or a socket.
