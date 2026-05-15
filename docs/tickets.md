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

## FS-034 - Define the first narrow area-loading contract

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The real-terrain workflow can load committed rasters, but the repository does not yet expose one explicit MVP contract for loading a user-selected DEM clip with clear provenance and clip status.

Proposed change:
- Define and implement one narrow area-loading contract around a DEM clip, optional boundary input, and explicit provenance metadata suitable for repeatable local screening workflows.

Constraints:
- Preserve the current MVP posture: one trustworthy local workflow first, not a broad ingestion subsystem.
- Keep GDAL usage narrow and optional beyond the current terrain path.
- Make CRS, nodata, and clip-status reporting explicit.

Acceptance criteria:
- One documented contract exists for loading a DEM clip with provenance and clip-status reporting.
- The example workflow can exercise that contract on a small committed or generated case.
- Failure modes for invalid bounds, CRS assumptions, or missing provenance are explicit.

Required tests:
- Coverage for the accepted area-loading inputs and at least one invalid contract case.

Required documentation updates:
- `docs/terrain_ingestion_contract.md`
- `examples/real_terrain/README.md`
- `README.md`

## FS-035 - Add curated real-terrain regression clips for planning cases

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The fixture set is useful but still narrow relative to the planning cases called out in the roadmap.

Proposed change:
- Add a small number of curated real-terrain regression clips that emphasize distinct screening situations such as drainage dominance, ponding, and nodata-heavy edges.

Constraints:
- Keep fixtures tiny, deterministic, and easy to review.
- Do not expand into a large data catalog.
- Each fixture should justify the behavior it is meant to cover.

Acceptance criteria:
- At least one additional committed fixture broadens regression coverage beyond the current set.
- Tests exercise the intended behavior of the new fixture.
- The example docs describe why each fixture exists.

Required tests:
- Real-terrain example or workflow regression coverage for the new fixtures.

Required documentation updates:
- `examples/real_terrain/README.md`

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
