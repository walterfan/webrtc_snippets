# Day 04 — SIP Dialogs

Implement `find_dialogs` for a header-only series of SIP messages. Identify
dialogs using Call-ID and the two endpoint tags. The result must recognize
messages in either direction and keep forked dialogs separate.

This is a bounded study of RFC 3261 Section 12. An initial request without
a To-tag is pre-dialog. A provisional response with a To-tag creates an
early dialog; a 2xx response confirms it. Different To-tags from a forked
INVITE produce different dialog identities.

Messages with missing Call-ID or tags are ignored. Do not implement dialog
usage authorization, route processing, target refresh, or transaction
timers.

## Acceptance test

Given one INVITE that receives two forked To-tags, when its responses and an
in-dialog request are grouped, then two dialogs are returned, the confirmed
dialog is not early, the other dialog remains early, and the reverse-direction
request is attached to the correct dialog.

Run:

```bash
ctest --test-dir build/basic -R Day04 --output-on-failure
```
