# Day 15 — RTP Header Codec

Implement `decode_header` and `encode_header` for the stated fixed 12-byte
V=2 subset (RFC 3550). Input is a packet span or an `RtpHeader` value;
output is the extracted fields or a 12-byte encoding. Example fields are
payload type, marker, sequence, timestamp, and SSRC. Reject packets shorter
than 12 bytes **before reading any byte**. Unsupported CSRC (`CC != 0`) and
header-extension (`X == 1`) packets are rejected even when the buffer is
long enough. Focus: bytes, fixed-width integers, V/P/X/CC bit operations,
and network byte order. Never open sockets, encrypt, or sleep.

## Requirements

### Wire layout (first 12 bytes)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

This exercise owns only that 12-byte subset. Sequence, timestamp, and SSRC
are unsigned big-endian (network byte order). Payload type is the low 7
bits of byte 1. Marker is the high bit of byte 1.

Byte 0 is the V/P/X/CC bitfield:

- `V` (bits 6–7) must be 2.
- `P` (bit 5) is observed for layout only; `RtpHeader` does not store it.
  `encode_header` always writes `P = 0`.
- `X` (bit 4) must be 0. Header-extension packets are unsupported.
- `CC` (bits 0–3) must be 0. CSRC lists are unsupported.

### `decode_header`

- If `packet.size() < 12`, return `std::nullopt` before indexing any byte.
- After the length check, read byte 0 and require `V == 2`.
- `CC != 0` (CSRC list present) → `std::nullopt`.
- `X == 1` (header extension present) → `std::nullopt`.
- Bytes after the first 12 are payload (or padding) and are ignored.
- On success, return `payload_type`, `marker`, `sequence`, `timestamp`,
  and `ssrc`.

### `encode_header`

- Return exactly 12 bytes: `V=2`, `P=0`, `X=0`, `CC=0`, then marker,
  7-bit payload type, and big-endian sequence / timestamp / SSRC.
- Mask `payload_type` to 7 bits. Do not emit CSRC or extension words.

### Out of scope

CSRC lists, header extensions, padding-length interpretation, payload
parsing, sockets, SRTP, and wall-clock behavior are **not** part of this
exercise.

## How to start

1. Read the public contract in `starter.hpp` (`RtpHeader`,
   `decode_header`, `encode_header`).
2. Implement both targets in `starter.cpp`; keep `Solution::run` as a
   render helper until the logic is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a
   finished solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day15` after wiring a real check.

## Acceptance fixtures

### Happy path — 12-byte V=2 header round-trips

- **Given** a valid 12-byte `V=2 P=0 X=0 CC=0` header (payload type,
  marker, sequence, timestamp, SSRC)
- **When** `decode_header` then `encode_header` run
- **Then** the documented fields match and the encoded form is 12 bytes

### Happy path — extra payload bytes are ignored

- **Given** a valid 12-byte header plus additional payload bytes
- **When** `decode_header` runs
- **Then** the header fields are returned and the extra bytes are not
  treated as CSRC or extension data

### Reject — truncated packet

- **Given** a packet shorter than 12 bytes (including empty)
- **When** `decode_header` runs
- **Then** the result is empty, and no byte was indexed

### Reject — wrong version

- **Given** a 12-byte packet whose `V` is not 2
- **When** `decode_header` runs
- **Then** the result is empty

### Reject — CSRC list

- **Given** a packet with `CC != 0`
- **When** `decode_header` runs
- **Then** the result is empty (CSRC is unsupported)

### Reject — header extension

- **Given** a packet with `X == 1`
- **When** `decode_header` runs
- **Then** the result is empty (extension is unsupported)
