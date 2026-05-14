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
- the next queued work is `FS-028` for intermediate runoff snapshots, unless another modeling priority overtakes it

## FS-028 - Add runoff result snapshots at selected times

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- Final-state output is not always enough for real scenarios because timing matters when comparing short intense storms against longer moderate ones.
- Without intermediate snapshots, scenario review is biased toward end-state inspection only.

Proposed change:
- Allow the real-terrain workflow to export a small number of selected time snapshots during a run.
- Keep selection simple, such as every N steps or a short explicit step list.

Constraints:
- Avoid building a full time-series storage system.
- Keep default behavior small enough for local runs and tests.

Acceptance criteria:
- A user can request intermediate output snapshots in a documented way.
- Snapshot filenames or metadata clearly identify simulation time.
- The implementation preserves deterministic ordering and stable export semantics.

Required tests:
- Add coverage for snapshot selection and output naming.
- Existing export and example tests continue to pass or are updated deliberately.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- export behavior notes where appropriate

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
