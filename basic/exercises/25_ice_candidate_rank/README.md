# Day 25 — ICE Candidate Ranking

Implement `rank_candidates`. Higher priority wins; ties preserve input order unless the stated component/transport tie breaker applies. Inputs are static candidates only. Test priority ties and component values. Focus: `<=>`, strong ordering, `stable_sort`.

## Requirements

- Rank static candidates by the documented priority and tie-break rules.
- Preserve input order for equivalent candidates with stable ordering.

## How to start

1. Implement comparison/ranking in `starter.cpp`.
2. Add priority ties, component, transport, and stability cases to `tests/acceptance_test.cpp`.
3. Run `ctest --test-dir build/basic -R Day25`.

## Acceptance test

- **Given** candidates with equal priority and distinct component/transport values.
- **When** they are ranked.
- **Then** documented tie breakers apply and truly equivalent candidates retain input order.
