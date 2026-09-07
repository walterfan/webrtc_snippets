# Day 20 — Stoppable Packet Queue

Implement `PacketQueue`. A blocked `pop` must wake when an item arrives,
`close()` is called, or its `std::stop_token` is requested. Example:
close an empty queue and receive no item. Avoid manual lock/unlock and
data races. Focus: mutexes, `condition_variable_any`, `jthread`,
`stop_token`.

## Requirements

### Storage

Private state is:

- a FIFO `std::queue<int>` with **fixed capacity 8** (`kCapacity`)
- `std::mutex` and `std::condition_variable_any`
- `closed_` (false until `close()`)

Use `condition_variable_any`, not `condition_variable`. The C++20
`wait(lock, stop_token, predicate)` overload is the stop path; do **not**
install a notify-only `std::stop_callback`. That callback pattern has a
lost-wakeup window between evaluating the predicate and entering `wait`.

Do not use a runtime-configurable size. `PacketQueue` is neither copyable
nor movable (the mutex and condition variable are not).

### `push`

- Use a `std::lock_guard` (or equivalent RAII lock). No manual
  `unlock`.
- If `closed_` is true or `packets_.size() == kCapacity`, return
  `false` and do not enqueue. `push` does **not** wait for space.
- Otherwise enqueue, `notify_one`, and return `true`.

### `pop`

- Use a `std::unique_lock` and
  `std::condition_variable_any::wait(lock, stop, predicate)`.
  No timed wait, no `sleep`, and no `stop_callback`.
- The predicate is `!packets_.empty() || closed_`. Stop is the
  `stop_token` argument, not a third clause you notify by hand.
- After `wait` returns: if the queue is non-empty, pop the front and
  return it (**data wins**, even if the queue is also closed or the
  token is already stopped). If the queue is empty, return
  `std::nullopt` (closed or stopped with no item).

`close()` does not discard queued items; later `pop` calls drain them,
then return empty.

`stop` on one `pop` does not set `closed_`. Another `pop` with a fresh
token may still wait for data.

An already-requested stop **before** `pop` is called must return
immediately: empty → `nullopt`; non-empty → the front item.

### `close`

Set `closed_ = true` under the mutex and `notify_all`. Further `push`
calls fail.

### Out of scope

Wall-clock timeouts, sockets, and busy-wait loops are **not** part of
this exercise. Day 22 snapshot consistency is a later task.

## How to start

1. Read the private queue, mutex, `condition_variable_any`, and
   `closed_` flag in `starter.hpp`.
2. Implement `push`, `pop`, and `close` in `starter.cpp`; keep
   `Solution::run` as a render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall.
4. Run `ctest --test-dir build/basic -R Day20` after wiring a real check.

## Acceptance fixtures

### Happy path — push wakes pop

- **Given** an empty queue and a blocked `pop`
- **When** `push(42)` succeeds
- **Then** `pop` returns `42`

### Close — empty

- **Given** `close()` on an empty queue
- **When** `pop` runs on the same thread with an unstopped token
- **Then** `pop` returns `nullopt`

### Stop — already requested, empty

- **Given** an empty queue and a `stop_source` that has already
  `request_stop()`ed
- **When** `pop(source.get_token())` runs on the same thread
- **Then** it returns `nullopt` without waiting for another thread to
  enter `wait`

### Stop — already requested, data wins

- **Given** a queued item and a token that is already stopped
- **When** `pop` runs
- **Then** it returns the item (stop does not close or discard)

### Close — drain

- **Given** a queued item, then `close()`
- **When** `pop` runs twice with an unstopped token
- **Then** the first call returns the item and the second is `nullopt`

### Bounded capacity

- **Given** eight successful pushes
- **When** a ninth `push` runs
- **Then** it returns `false` and the queue still has eight items

### Same-thread drain

- **Given** `push(1)` then `pop` with an unstopped token
- **When** both run on one thread
- **Then** `pop` returns `1` without blocking
