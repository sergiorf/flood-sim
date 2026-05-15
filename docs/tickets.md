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

## FS-037 - Allow reviewed scenario files to reference rainfall profiles

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The real-terrain example can now replay a step-varying rainfall profile from the CLI, but reviewed multi-scenario CSV contracts still only describe constant-intensity events.

Proposed change:
- Extend the narrow scenario-file contract so one reviewed scenario row can reference an external rainfall-profile CSV without relying on ad hoc CLI assembly.

Constraints:
- Keep the scenario-file contract explicit and local to the example workflow.
- Do not build a generalized scheduler or nested config format.
- Preserve deterministic reporting and export metadata for both uniform and profile-driven runs.

Acceptance criteria:
- A scenario file can describe at least one profile-driven rainfall event through a documented path field or equivalent narrow contract.
- Batch and single-scenario runs preserve enough metadata to distinguish uniform and profile-driven events safely.
- Tests cover both valid and invalid reviewed profile references.

Required tests:
- Real-terrain workflow regression coverage for scenario-file-driven rainfall profiles.

Required documentation updates:
- `examples/real_terrain/README.md`
- `README.md`

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
