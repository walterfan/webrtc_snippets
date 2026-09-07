# Day 07 — Registration Bindings

Implement `RegistrationStore`. Input is a `RegisterUpdate` for one Address-of-Record
(AOR); output is the active `ContactBinding` list returned by `lookup`. The AOR
is the stable identity (`sip:alice@atlanta.com`). Each Contact URI is a current
device binding under that AOR. Focus: owning containers, create/replace/remove
semantics, and deterministic lookup order. Never open sockets, resolve DNS, or
calculate Digest credentials.

## Requirements

- Treat `RegisterUpdate::aor` as the lookup key and each `ContactBinding::uri`
  as the binding key within that AOR.
- A Contact with `expires > 0` creates or replaces the binding for the same
  AOR and Contact URI (refresh may change `expires` and `q_value`).
- A Contact with `expires == 0` removes that binding only.
- `wildcard_remove == true` clears every binding for the AOR (teaching form of
  `Contact: *` with expiry zero). Ignore the `contacts` vector in that case.
- `apply` returns `false` for an empty AOR and leaves the store unchanged;
  otherwise it returns `true` after applying the update.
- `lookup` returns only active bindings for the requested AOR as owned values
  (no internal references). Order is deterministic: higher `q_value` first;
  equal `q` values sort by URI ascending. Unknown or empty AORs yield an empty
  vector.
- Updates for one AOR must not affect bindings stored under a different AOR.

## Out of scope

RFC 5626 Outbound and RFC 5627 GRUU are binding-metadata topics for later
reading, not part of this store. REGISTER Digest `401`/`407` challenges are a
Day 08 response concern; do **not** compute Digest responses here.

## How to start

1. Read the public contract in `starter.hpp` (`ContactBinding`,
   `RegisterUpdate`, `RegistrationStore`).
2. Implement `RegistrationStore::apply` and `lookup` in `starter.cpp`; keep
   `Solution::run` as a render helper until the store is ready.
3. Use progressive hints in `tips.md` if you stall; do not copy a finished
   solution from elsewhere.
4. Run `ctest --test-dir build/basic -R Day07` after wiring a real check.

## Acceptance fixtures

Use AOR `sip:alice@atlanta.com` unless a fixture names another one.

### Happy path — three Contacts under one AOR

- **Given** desktop, mobile, and browser Contacts registered with positive
  expiry and distinct `q_value` values (for example `100`, `50`, `10`)
- **When** `lookup` runs for Alice's AOR
- **Then** all three bindings are present, ordered by descending `q_value`

### Happy path — refresh one URI

- **Given** the three-Contact store, then a second update that refreshes only
  the desktop URI with a new `expires` (and optionally a new `q_value`)
- **When** `lookup` runs
- **Then** the desktop binding shows the refreshed fields; mobile and browser
  are unchanged; binding count remains three

### Happy path — remove one Contact

- **Given** the three-Contact store, then an update with the mobile URI and
  `expires=0`
- **When** `lookup` runs
- **Then** only desktop and browser remain

### Happy path — wildcard removal

- **Given** any non-empty set of Contacts for Alice, then
  `wildcard_remove == true`
- **When** `lookup` runs for Alice's AOR
- **Then** the result is empty

### Reject — empty AOR

- **Given** an update whose `aor` is empty
- **When** `apply` runs
- **Then** it returns `false` and the store is unchanged

### Isolation — independent AOR

- **Given** Contacts under `sip:alice@atlanta.com` and a separate update for
  `sip:bob@biloxi.com`
- **When** each AOR is looked up
- **Then** Alice's and Bob's bindings remain independent
