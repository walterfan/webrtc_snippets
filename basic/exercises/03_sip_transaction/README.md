# Day 03 — SIP Transactions

Implement `find_transactions` for a header-only series of SIP messages.
Group messages using the top Via `branch`, `sent-by`, and CSeq method. A
Call-ID identifies a call, not a transaction, so two branches under one
Call-ID must remain separate.

This is a bounded study of RFC 3261 Sections 17.1.3 and 17.2.3. Include
retransmissions, provisional/final responses, the CANCEL transaction, and
the non-2xx INVITE ACK association. An ACK for a 2xx response is not part of
the INVITE transaction.

The shared parser accepts header-only messages separated by `\r\n\r\n`.
Malformed Via/CSeq fields are ignored by the grouping algorithm. Do not
implement transport, timers, retransmission I/O, or a complete SIP stack.

## Acceptance test

Given two INVITEs with different branches, responses, a CANCEL, and a
non-2xx ACK, when the series is grouped, then the branches are separated,
the ACK joins the non-2xx INVITE, and CANCEL remains its own transaction.

Run:

```bash
ctest --test-dir build/basic -R Day03 --output-on-failure
```
