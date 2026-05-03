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

## FS-012 - Make nodata and terrain-domain handling explicit

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- Real terrain data introduces nodata cells, clipped extents, and edge behavior that the toy-grid phase did not need to handle explicitly.
- If those rules stay implicit, imported runs will be hard to trust or compare.

Proposed change:
- Define and implement how nodata cells and domain edges behave in the first real-terrain workflow.
- Align the behavior with the current explicit-boundary approach where possible.

Constraints:
- Keep the first rule set simple and easy to explain.
- Do not attempt to solve every GIS edge case in the first Phase 2 pass.

Acceptance criteria:
- Nodata handling rules are explicit in code and docs.
- Domain-edge behavior for imported terrain is stated clearly.
- Tests cover at least one representative nodata or clipped-domain case.

Required tests:
- Focused nodata or domain-edge tests.
- Any simulation-adjacent behavior changes should be covered in `core/tests` or equivalent small tests.

Required documentation updates:
- `docs/simulation_model.md`
- `docs/architecture.md`

## FS-013 - Add the first real-terrain example workflow

Status: Todo
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

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.

## Done
