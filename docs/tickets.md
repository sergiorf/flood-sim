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

Next recommended Phase 2 step:
- `FS-026` is the next active task now that `FS-031` is done.

Phase 2 stop rule:
- after the minimum comparison scaffolding is in place, priority should shift to the first realism-bearing model changes rather than broader scenario orchestration
- `FS-023` completed that minimum comparison scaffolding pass, so the next priority is realism-bearing model work

## FS-024 - Add batch scenario execution for one terrain clip

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- Running one scenario at a time is enough for smoke tests, but it is weak for practical real-scenario exploration.
- Early scenario comparison needs a tiny repeatable batch path before the project takes on a richer UI or orchestration layer.

Proposed change:
- Add a minimal batch mode that runs several named scenarios over the same terrain clip and writes outputs to separate deterministic paths.
- Keep the interface narrow and local to the example or a small helper.

Constraints:
- Do not add job scheduling, concurrency, or a config-file hierarchy in this ticket.
- Preserve a simple single-scenario mode.

Acceptance criteria:
- A user can run multiple documented named scenarios in one invocation.
- Output files are deterministic and clearly associated with their scenario names.
- Failures for one scenario are reported clearly.

Required tests:
- Add or update smoke coverage for a small batch run.
- Add narrow tests for scenario list parsing if nontrivial logic is introduced.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`

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

## FS-026 - Add terrain-derived scenario fixtures for regression coverage

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- Real-scenario work will drift unless the repository carries a few small deterministic scenario fixtures beyond the original sample path.
- Depending on one example clip and one run shape is too narrow for Phase 2 hardening.

Proposed change:
- Add a tiny curated set of regression fixtures or fixture descriptors for real-terrain runs.
- Cover at least one case with nodata influence and one case with a clear drainage pattern.

Constraints:
- Keep fixture size small enough for local development and repository storage.
- Avoid creating a large sample-data archive.

Acceptance criteria:
- The repository includes at least two named deterministic real-terrain scenario fixtures.
- Tests or smoke workflows exercise the fixtures reproducibly.
- Fixture intent is documented so later contributors understand why each case exists.

Required tests:
- Add or update example and regression coverage to use the new fixtures.
- Existing ingestion and export tests continue to pass.

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

## FS-029 - Add one documented real-scenario comparison walkthrough

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The repository can gain scenario mechanics without becoming easier for a human to use.
- Before Phase 2 is considered hardened, the docs should show one end-to-end comparison workflow on a real clip that another developer can rerun exactly.

Proposed change:
- Document one canonical scenario-comparison walkthrough using the real-terrain example, named scenarios, and the current output artifacts.
- Keep the walkthrough focused on reproducibility and interpretation rather than visual polish.

Constraints:
- Do not require external web tooling or notebook infrastructure.
- Keep the walkthrough aligned with the actual committed example data.

Acceptance criteria:
- A new contributor can run the documented comparison workflow and understand the produced outputs.
- The walkthrough names the exact commands and expected artifacts.
- The docs clearly state the modeling limits of the comparison.

Required tests:
- Add or update a smoke test for the documented walkthrough if practical.
- Call out any remaining manual-only verification explicitly if full automation is not practical yet.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- `docs/roadmap.md` if the Phase 2 wording should reflect the hardened scenario path

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
