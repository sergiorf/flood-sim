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

Phase 2 stop rule:
- after the minimum comparison scaffolding is in place, priority should shift to the first realism-bearing model changes rather than broader scenario orchestration
- `FS-023` completed that minimum comparison scaffolding pass
- `FS-030` completed the first clipped-terrain edge-outflow pass
- `FS-031` completed the first simple rainfall-loss control pass

Planning note:
- deterministic real-terrain regression fixtures are now in place for nodata influence and clear drainage coverage
- `FS-029` completed the canonical real-scenario comparison walkthrough pass
- `FS-024` completed the first batch execution pass for one terrain clip
- `FS-025` completed the first batch comparison artifact pass
- `FS-027` completed the first external scenario-definition format pass
- `FS-028` completed the first intermediate runoff snapshot pass
- the next planning surface is the product MVP task list in `docs/roadmap.md`, with bias toward more terrain fixtures, map loading, visualization, and the next realism-bearing model change

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
