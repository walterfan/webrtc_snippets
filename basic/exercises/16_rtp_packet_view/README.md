# Day 16 — Zero-Copy RTP Packet View

Implement `RtpPacketView::from`, `header`, and `payload`. Input is
caller-owned immutable bytes; the view stores a `std::span<const std::byte>`
that aliases that storage and must not extend its lifetime. A buffer of at
least 12 bytes is a valid header-plus-payload view; shorter data is
rejected. Focus: C++20 `span`, const correctness, lifetimes.

## Requirements

### Storage and lifetime

- After `from` succeeds, the view holds a **caller-owned**
  `std::span<const std::byte>` over the same bytes. It does not copy the
  packet and does not own the buffer.
- `header()` and `payload()` return subspans of that stored span. They
  become dangling if the caller’s buffer is destroyed or moved-from in a
  way that releases the storage.
- Do not allocate, do not return a span into a temporary, and do not
  extend lifetime with `shared_ptr` or a copied `vector`.

### `from`

- If `bytes.size() < 12`, return `std::nullopt` **before** forming a
  view and **before** indexing any byte.
- If `bytes.size() >= 12`, construct the view over the **entire** input
  span (header and any trailing payload). Extra bytes after offset 12 are
  payload, not a reason to reject.
- This exercise does **not** re-validate RTP version, padding, CSRC, or
  extensions. Day 15 owns that subset. Here the only gate is the 12-byte
  header boundary.

### `header` / `payload`

- `header()` is exactly 12 bytes: `bytes.first(12)`.
- `payload()` is the remainder: `bytes.subspan(12)` (empty when the
  input was exactly 12 bytes).
- Both spans must stay inside the original buffer: `header().size() +
  payload().size() == stored.size()`, and their data pointers must equal
  `stored.data()` and `stored.data() + 12`.

### Out of scope

Sockets, SRTP, wall-clock waits, and Day 15’s V/P/X/CC checks are **not**
part of this exercise.

## How to start

1. Read the public contract and the private `bytes_` span in
   `starter.hpp`. Construct through the private constructor from `from`.
2. Implement `from`, `header`, and `payload` in `starter.cpp`; keep
   `Solution::run` as a render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a
   finished solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day16` after wiring a real check.

## Acceptance fixtures

### Happy path — header plus payload

- **Given** an immutable 16-byte buffer (12-byte header + 4 payload bytes)
- **When** `RtpPacketView::from` succeeds
- **Then** `header().size() == 12`, `payload().size() == 4`, and both
  spans alias the caller buffer (same `data()` base)

### Happy path — header only

- **Given** a 12-byte buffer
- **When** a view is created
- **Then** `payload()` is empty and `header()` covers all 12 bytes

### Reject — short buffer

- **Given** an empty span or an 11-byte buffer
- **When** `from` runs
- **Then** the result is empty and no byte was indexed

### Lifetime — caller storage

- **Given** a successful view
- **When** the caller still owns the original buffer
- **Then** header/payload reads see those bytes; the view must not keep
  the buffer alive after the caller releases it
