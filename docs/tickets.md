# Tickets

This file is the local ticket database for the repository.

Use it instead of GitHub issues when the coding environment is intentionally isolated from network and account access.

## Conventions

- Keep ticket IDs stable: `FS-001`, `FS-002`, and so on.
- Keep each ticket small enough to finish in one focused milestone.
- Move completed tickets to the `Done` section instead of deleting them.
- Update acceptance criteria, tests, and docs notes when scope changes.

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

## FS-005 - Clarify rainfall scenario and time-step assumptions

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- Phase 1 includes rainfall accumulation, but the assumptions around intensity, duration, and per-step application need to be explicit.
- Ambiguity here will make Phase 5 scenario work harder later.

Proposed change:
- Define the meaning of the rainfall scenario inputs used by the MVP and how they interact with `time_step_seconds`.
- Align naming, docs, and tests around that contract.

Constraints:
- Do not expand into full scenario modeling yet.
- Preserve a minimal API suitable for the toy-grid prototype.

Acceptance criteria:
- The rainfall input contract is explicit in code comments or docs.
- Tests show the expected relationship between rainfall settings and accumulated depth over one or more steps.
- Any misleading names or comments are corrected.

Required tests:
- Step-duration-sensitive rainfall tests.
- Multi-step accumulation tests.

Required documentation updates:
- `docs/simulation_model.md`
- `docs/architecture.md`

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.

## Done
