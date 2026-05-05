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

## FS-016 - Preserve georeferencing in exported real-terrain outputs

Status: Done
Owner: Unassigned
Priority: P1

Problem:
- The current CSV export is self-describing at the grid level, but it still loses the real-world placement metadata preserved during terrain ingestion.
- Without origin and CRS metadata in exported real-terrain outputs, downstream inspection and later map-aligned workflows remain weaker than the Phase 2 roadmap promises.

Proposed change:
- Extend the exported real-terrain output contract so it preserves the minimum georeferencing metadata already available in `TerrainRaster`.
- Keep the format lightweight and explicit rather than introducing a heavy GIS export dependency at this stage.
- Make the export behavior for terrain-derived runs easy to validate and hard to misuse.

Constraints:
- Preserve the existing simple export posture unless there is a strong reason to split toy-grid and real-terrain output modes.
- Do not add raster reprojection or full GIS writer complexity in this ticket.
- Keep the metadata contract stable and documented.

Acceptance criteria:
- Real-terrain exports preserve enough metadata to identify raster dimensions, cell size, origin, and CRS.
- The export format documents when georeferencing metadata is present and what each field means.
- The real-terrain example emits the documented metadata in a reproducible way.
- Downstream parsing or inspection code can read the added metadata without ambiguity.

Required tests:
- Add or update tests that pin the real-terrain export metadata contract exactly.
- Add failure or edge-case coverage if the implementation must handle missing optional georeferencing metadata.
- Existing export consumer tests must continue to pass or be updated deliberately with the new contract.

Required documentation updates:
- `README.md`
- `docs/architecture.md`
- export-format documentation in example or model docs as appropriate

## FS-017 - Make the real-terrain example configurable for repeatable scenario runs

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The first real-terrain example currently proves the workflow, but the simulation inputs are hardcoded.
- That makes it weaker as a practical hardening tool for local exploration, regression checks, and early user-facing scenario demonstrations.

Proposed change:
- Let the real-terrain example accept a small explicit set of scenario parameters through the CLI.
- Keep the interface narrow: enough to vary rainfall and step settings without drifting into a full scenario-management system.
- Document one or two canonical example invocations so the workflow stays reproducible.

Constraints:
- Keep the CLI small and easy to understand.
- Do not introduce a new config-file system in this ticket.
- Preserve a deterministic documented default path so the example remains usable in tests and docs.

Acceptance criteria:
- A user can run the real-terrain example with explicit rainfall and time-step settings from the command line.
- The example still supports one documented default invocation for reproducible smoke testing.
- Invalid CLI inputs fail clearly.
- The workflow remains simple enough for local testing and future demo use.

Required tests:
- Add or update smoke tests to cover the documented default path.
- Add narrow coverage for CLI parsing or validation if nontrivial logic is introduced.
- Existing example workflow tests continue to pass with the new interface.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- any test or usage notes affected by the CLI contract

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.

## Done

## FS-015 - Add doctest and migrate the C++ test suite

Status: Done
Owner: Unassigned
Priority: P2

Problem:
- The current C++ test suite uses a custom handwritten harness that is still workable, but it now adds friction as coverage grows.
- Failure output, assertion ergonomics, and test organization will become harder to maintain through later Phase 2 and Phase 3 work.

Proposed change:
- Add `doctest` as the project’s lightweight C++ test framework dependency.
- Migrate the existing C++ tests from the custom harness to `doctest` while preserving current simulation and ingestion coverage.
- Keep the existing Python smoke tests for example workflows unless there is a strong reason to replace them.

Constraints:
- Keep dependencies minimal and aligned with the repository’s current lightweight posture.
- Do not change simulation semantics as part of the migration.
- Keep CI and local build/test commands straightforward.

Acceptance criteria:
- The repository builds the C++ test target with `doctest`.
- The current handwritten assertion helpers and manual test runner are removed from the core C++ tests.
- Existing C++ simulation, export, terrain-contract, and GDAL-ingestion tests are migrated without losing coverage intent.
- `ctest --test-dir build --output-on-failure` still runs cleanly with the migrated suite.

Required tests:
- The migrated C++ suite must cover the same current behaviors at minimum.
- Any framework integration code or custom `doctest` configuration should receive narrow coverage only if it introduces nontrivial logic.

Required documentation updates:
- `README.md`
- build or testing notes if commands or dependencies change

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
