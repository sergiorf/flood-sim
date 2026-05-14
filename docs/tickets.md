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
- the next queued work is the comparison artifact path starting with `FS-025`

## FS-025 - Add scenario-comparison summaries across batch runs

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- Batch execution alone still leaves users reading multiple outputs manually.
- The first useful real-scenario workflow needs one compact comparison artifact before any map-based viewer exists.

Proposed change:
- Produce a small comparison table across batch runs with shared metrics such as maximum depth and wet-cell count.
- Keep the artifact text-based or CSV-based for now.

Constraints:
- Reuse metrics already defined for single runs where possible.
- Avoid introducing visualization or dashboard work in this ticket.

Acceptance criteria:
- Batch runs produce one deterministic comparison artifact.
- The comparison clearly identifies each scenario and its summary values.
- The output is documented well enough for manual inspection or future scripting.

Required tests:
- Add narrow tests for comparison-table generation.
- Existing batch workflow tests continue to pass.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`

## FS-027 - Add a simple scenario-definition file format

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- Named presets in code are a good bridge, but they will become limiting once users want to maintain a small set of shareable real scenarios.
- Keeping all scenario definitions compiled into the example is friction for iteration and review.

Proposed change:
- Add a minimal human-editable scenario-definition format such as a narrow key-value or CSV contract.
- Support only the fields already stabilized by the scenario config contract.

Constraints:
- Keep the format trivial to parse with the standard library.
- Do not add YAML, JSON schema tooling, or external parsing dependencies unless explicitly justified later.

Acceptance criteria:
- A user can define at least one scenario outside the binary and run it reproducibly.
- Invalid scenario files fail with clear messages.
- The file contract is documented and intentionally small.

Required tests:
- Add parsing and validation coverage for valid and invalid files.
- Existing CLI scenario paths continue to work.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- format notes in `docs/architecture.md` if needed

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
