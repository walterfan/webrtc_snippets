# Day 23 — SDP Media Sections

Implement `parse_media_sections` for a narrow line-oriented subset. Example: `m=audio ...` followed by `a=rtpmap:...` becomes one audio section. Reject malformed media lines and preserve only defined `a=` attributes. Focus: C++20 ranges, views, `string_view`; not an RFC-complete SDP parser.

## Requirements

- Parse only the stated line-oriented `m=`/`a=` subset.
- Reject malformed media lines and do not retain unsupported attributes.

## How to start

1. Split SDP lines with views/ranges in `starter.cpp`.
2. Add audio, video, malformed, and unsupported-attribute cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day23`.

## Acceptance test

- **Given** audio and video `m=` lines with following `a=rtpmap` attributes.
- **When** the SDP is parsed.
- **Then** two media sections are returned with their defined attributes; malformed `m=` input is rejected.
