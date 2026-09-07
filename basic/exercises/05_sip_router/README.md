# Day 05 — SIP Route Paths

Implement `find_route_paths` for several dialogs represented by a
header-only message series. Build route sets from Record-Route and apply
Route headers to later in-dialog requests. Preserve route order.

This is a bounded study of RFC 3261 Sections 12.2.1.1, 12.2.2.1, and 16.12.
With `;lr`, use loose routing and keep the request URI as the final
destination. Without `;lr`, apply the limited strict-routing normalization
described in the exercise. Use Contact as the direct destination when no
route is available.

Do not perform DNS, transport selection, proxy forwarding, socket I/O, or
complete URI grammar validation. Messages without a valid dialog identity
do not produce a route path.

## Acceptance test

Given two dialog route sets containing loose and strict proxies, when the
in-dialog requests are processed, then both route paths preserve their
order, append the final destination correctly, and exclude unrelated
messages.

Run:

```bash
ctest --test-dir build/basic -R Day05 --output-on-failure
```
