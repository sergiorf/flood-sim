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

## FS-014 - Apply Phase 2 hot-path cleanup before larger terrain runs

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- The current simulation implementation is clear and correct, but it still carries obvious per-step overhead that will matter more once real terrain clips and longer runs become common.
- Waiting too long to address the simplest hot-path waste will make early Phase 2 performance harder to interpret.

Proposed change:
- Apply only the low-risk performance cleanups that improve the current hot path without changing model semantics.
- Focus on removing avoidable allocation and container overhead before considering deeper optimization work.

Constraints:
- Preserve the current simulation behavior exactly.
- Keep the code readable and testable.
- Do not introduce speculative large-scale optimization architecture yet.

Acceptance criteria:
- Per-step temporary neighbor storage no longer performs avoidable dynamic allocation.
- The step update buffer is reused or otherwise avoids unnecessary per-step allocation churn.
- Any fast-path access changes preserve the current documented semantics.
- The change is accompanied by at least a small before/after rationale in code comments or docs if the implementation becomes less obvious.

Required tests:
- Existing simulation tests continue to pass unchanged.
- Any new helper abstractions for the hot path receive narrow coverage if they introduce nontrivial logic.

Required documentation updates:
- None required unless implementation tradeoffs need a brief note in `docs/architecture.md`

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.

## Done

## FS-013 - Add the first real-terrain example workflow

Status: Done
Owner: Unassigned
Priority: P2

Problem:
- Even after ingestion exists, the repository still needs one concrete end-to-end example showing a real terrain clip flowing through the system.
- Without that example, Phase 2 remains technically incomplete and hard to demonstrate.

Proposed change:
- Add one small real-terrain example that loads terrain, runs rainfall, and exports results.
- Keep the example focused on proving the first real-terrain milestone rather than visualization polish.

Constraints:
- Use a small input example suitable for local development.
- Keep the workflow reproducible and easy to run.

Acceptance criteria:
- The repository contains one documented real-terrain example workflow.
- The example exercises ingestion, simulation, and export together.
- The example output is suitable for later visualization or inspection work.

Required tests:
- Example smoke-test guidance or automation where practical.
- Any helper code introduced for the example should have narrow coverage if it is nontrivial.

Required documentation updates:
- `README.md`
- example-specific docs
- `docs/roadmap.md` if the milestone wording needs refinement
