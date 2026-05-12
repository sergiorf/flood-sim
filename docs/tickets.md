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
- `FS-023` completed that minimum comparison scaffolding pass, so the next priority is realism-bearing model work

Planning note:
- deterministic real-terrain regression fixtures are now in place for nodata influence and clear drainage coverage
- if additional workflow hardening is still wanted before model work, `FS-029` is the remaining higher-priority documentation task

## FS-030 - Add clipped-terrain edge outflow behavior

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The current model still leans on closed-edge behavior as the default mental model, which can trap water unrealistically on clipped real-terrain rasters.
- Real-terrain runs need one explicit edge treatment that better approximates water leaving the simulated domain so scenario comparisons are not dominated by artificial perimeter ponding.

Proposed change:
- Add one narrow outflow behavior for clipped terrain that allows water to leave eligible boundary cells in a deterministic, documented way.
- Keep the change small by expressing it through the existing simulation and example configuration path rather than introducing a broad boundary-policy framework.

Constraints:
- Preserve the current simple raster kernel and avoid introducing channel routing, full hydraulic boundary conditions, or external dependencies.
- Keep existing closed-boundary behavior available and unchanged for tests and toy examples.
- Define the new behavior in physically modest terms and document clearly what it approximates and what it does not.

Acceptance criteria:
- A user can run the real-terrain example with the new edge behavior through the existing scenario or CLI configuration path.
- On a small deterministic drainage fixture, the new mode reduces retained water relative to closed boundaries in a way that is stable across repeated runs.
- Water loss occurs only through documented boundary conditions; interior valid cells and nodata masking semantics remain unchanged.
- Exported metadata and run summaries identify which boundary behavior was used so comparisons remain unambiguous.

Required tests:
- Add core simulation coverage for a small raster where boundary outflow changes the final retained-water result relative to closed boundaries.
- Add or update a real-terrain regression test using the committed drainage fixture to show stable, deterministic behavior for the new mode.
- Existing closed-boundary tests continue to pass unchanged.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- `docs/simulation_model.md`
- `docs/architecture.md` if configuration or export contracts change

## FS-031 - Add a simple infiltration or rainfall-loss control

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- Rainfall currently maps too directly to surface water accumulation, which limits the practical meaning of real-terrain screening runs.
- The project needs one minimal loss mechanism so scenario comparisons can represent at least a first-pass difference between gross rainfall and surface runoff.

Proposed change:
- Add one deliberately simple loss representation, such as a constant infiltration rate or other bounded rainfall-loss term, applied through the existing simulation configuration.
- Keep the model uniform and local for the first pass so the behavior is easy to test, explain, and compare before considering spatially varying soils or land cover.

Constraints:
- Do not introduce calibration-heavy hydrology, soil databases, evapotranspiration modeling, or spatial preprocessing in this ticket.
- Preserve current behavior as a documented zero-loss or disabled-loss case so existing examples and tests can remain valid.
- Keep parameter semantics explicit in physical units and guard against configurations that can remove more water than is available from rainfall or surface storage.

Acceptance criteria:
- A user can enable the loss model in the real-terrain workflow using a documented parameter with clear units and valid bounds.
- With loss disabled, results remain backward-compatible with the current behavior.
- With a positive loss value, a deterministic test case shows reduced accumulated surface water relative to the zero-loss case.
- The implementation documents whether the loss applies to incoming rainfall only, ponded surface water, or both, and exports enough metadata to make that choice visible in outputs.

Required tests:
- Add unit coverage for parameter validation and for the zero-loss compatibility case.
- Add a deterministic simulation test showing that a positive configured loss reduces retained water without producing negative depths.
- Add or update a real-terrain example or regression test that exercises the documented loss parameter path.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- `docs/simulation_model.md`
- `docs/architecture.md` if scenario or export contracts change

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
