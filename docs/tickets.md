# Tickets

This file is the active local ticket queue for the repository.

Use it instead of GitHub issues when the coding environment is intentionally isolated from network and account access.

## Conventions

- Keep ticket IDs stable: `FS-001`, `FS-002`, and so on.
- Keep each ticket small enough to finish in one focused milestone.
- Remove completed tickets from this file once they no longer affect active planning.
- Update acceptance criteria, tests, and docs notes when scope changes.
- Keep the queue ordered so the next likely task appears near the top of `Todo`.

## Ticket Template

```text
## FS-XXX - Short title

Status: Todo | In Progress | Blocked | Done
Owner: Unassigned
Priority: P1 | P2 | P3

Problem:
- What is wrong or missing?

Proposed change:
- What should be built or changed?

Constraints:
- Important technical or product limits.

Acceptance criteria:
- Observable condition 1.
- Observable condition 2.

Required tests:
- Test coverage that must be added or updated.

Required documentation updates:
- Docs that must change with the implementation.
```

## Todo

Planning note:
- The minimum deterministic real-terrain comparison path is in place.
- New tickets should bias toward interpretation, trusted area loading, curated fixtures, and lightweight visualization over broader orchestration.

## FS-036 - Add the next realism-bearing hydrology benchmarked against fixtures

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The repository has the first open-boundary and simple runoff-loss controls, but it still needs the next realism-bearing model change that improves practical screening value rather than workflow shape alone.

Proposed change:
- Implement one additional narrowly scoped hydrology improvement and measure its effect against committed real-terrain fixtures.

Constraints:
- Keep the model explicit and testable.
- Prefer one explainable physical approximation over several loosely justified toggles.
- Record what the new approximation still does not represent.

Acceptance criteria:
- One new hydrology behavior is implemented behind clear scenario or model inputs.
- Tests constrain the new behavior on at least one toy or real-terrain fixture.
- Documentation explains why the change improves interpretation and what limitations remain.

Required tests:
- Unit or workflow regression coverage for the selected hydrology improvement.

Required documentation updates:
- `docs/simulation_model.md`
- `examples/real_terrain/README.md` if the real-terrain workflow exposes the new control

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
