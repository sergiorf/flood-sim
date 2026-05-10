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

## FS-018 - Add terrain-window clipping for repeatable real-area scenarios

Status: Done
Owner: Unassigned
Priority: P1

Problem:
- The repository can load one committed tiny terrain clip, but it still lacks a lightweight way to define multiple named real-area scenario windows from a larger source raster.
- Without explicit clipping support, scenario work stays tied to one baked sample instead of a repeatable workflow over real locations.

Proposed change:
- Add a narrow terrain-window selection path that clips a rectangular area from a source DEM or GeoTIFF into a validated `TerrainRaster`.
- Keep the first interface simple: bounding rows and columns or an explicit pixel window before considering map-coordinate clipping.

Constraints:
- Preserve the current narrow ingestion contract.
- Avoid introducing GIS reprojection or heavy preprocessing orchestration in this ticket.
- Keep the output deterministic for tests and examples.

Acceptance criteria:
- A caller can request a smaller terrain window from a larger raster source.
- Invalid or out-of-range windows fail clearly.
- The clipped result preserves valid-mask, cell size, and available georeferencing metadata correctly.

Required tests:
- Add coverage for successful clipping and invalid window bounds.
- Existing terrain-ingestion tests continue to pass.

Required documentation updates:
- `README.md`
- `docs/terrain_ingestion_contract.md`
- example usage notes if a new workflow is exposed

## Todo

Next recommended Phase 2 step:
- `FS-022` is the next active Phase 2 task now that `FS-021` is done.

Phase 2 stop rule:
- after the minimum comparison scaffolding is in place, priority should shift to the first realism-bearing model changes rather than broader scenario orchestration
- in practice, that means `FS-021`, `FS-022`, and `FS-023` are the likely end of the current scaffolding pass unless one of them proves unnecessary

## FS-019 - Support explicit nodata policy reporting in terrain ingestion

Status: Done
Owner: Unassigned
Priority: P1

Problem:
- Real scenarios become hard to trust when nodata handling is implicit.
- The current path masks invalid cells correctly, but it does not yet make nodata behavior visible enough for scenario review and debugging.

Proposed change:
- Expose a small ingestion report or metadata structure describing nodata presence, valid-cell count, and any clipping loss.
- Keep the reporting path lightweight and machine-readable.

Constraints:
- Do not change simulation semantics for valid cells.
- Do not introduce a large logging framework.

Acceptance criteria:
- Ingestion returns or emits a narrow summary of nodata and valid-domain handling.
- Real-terrain example output or logs make the imported domain easier to inspect.
- Missing or inconsistent nodata metadata is handled explicitly rather than silently.

Required tests:
- Add tests for rasters with nodata and rasters without nodata metadata.
- Update example smoke coverage if the user-facing output changes.

Required documentation updates:
- `README.md`
- `docs/terrain_ingestion_contract.md`

## FS-020 - Add a scenario input struct for real-terrain runs

Status: Done
Owner: Unassigned
Priority: P1

Problem:
- Real-terrain runs currently rely on a small CLI surface, but the scenario information still lives as loose parameters rather than one explicit contract.
- That makes it harder to extend toward named real scenarios, reproducible comparisons, and batch execution.

Proposed change:
- Introduce a narrow `ScenarioConfig` or equivalent value object for real-terrain runs.
- Include only the fields already needed or immediately planned: rainfall intensity, duration or step count, time step, and output path or label.

Constraints:
- Keep the object minimal and focused on Phase 2 needs.
- Do not introduce a full scenario-management subsystem yet.

Acceptance criteria:
- The real-terrain workflow uses an explicit scenario configuration object instead of ad hoc parameter passing.
- The object can support both default example runs and explicit user-provided values.
- Validation failures are clear and local to scenario parsing or construction.

Required tests:
- Add narrow tests for scenario validation or parsing if nontrivial logic is introduced.
- Existing example workflow tests continue to pass.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- `docs/architecture.md` if responsibilities shift materially

## FS-021 - Add named rainfall presets for repeatable real scenarios

Status: Done
Owner: Unassigned
Priority: P1

Problem:
- Real-scenario work needs a few reproducible rainfall cases that can be discussed and rerun consistently.
- Requiring users to enter raw numeric values every time weakens repeatability and comparison.

Proposed change:
- Add a small set of named rainfall presets for the real-terrain example, such as `baseline`, `intense_short`, and `long_moderate`.
- Keep presets as code or a tiny static table rather than creating a full external catalog format yet.

Constraints:
- Preserve explicit CLI overrides for direct experimentation.
- Do not imply scientific calibration beyond clearly documented toy-scenario intent.

Acceptance criteria:
- Users can select at least two documented named rainfall presets.
- The preset definitions are explicit and easy to inspect.
- Preset selection and direct numeric overrides cannot silently conflict.

Required tests:
- Add coverage for preset lookup and invalid preset names.
- Update smoke tests to use at least one named preset path.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`

## FS-022 - Export run metadata with scenario identity and timing

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- Exported depth grids now preserve georeferencing, but they still do not carry enough scenario identity to compare multiple real runs safely.
- Once several real scenarios exist, unlabeled CSV outputs will become easy to confuse.

Proposed change:
- Extend the export metadata preamble with a narrow set of run fields such as scenario name, rainfall intensity, time step, and total simulated duration.
- Keep the CSV contract simple and backward-compatible where practical.

Constraints:
- Avoid turning the export into a general provenance system.
- Preserve the current row-wise table layout.

Acceptance criteria:
- Real-terrain exports include documented scenario and timing metadata.
- Export consumers can parse the added metadata without ambiguity.
- The default example produces deterministic metadata values in tests.

Required tests:
- Add or update tests that pin the new metadata contract.
- Existing export consumer tests are updated deliberately if needed.

Required documentation updates:
- `README.md`
- `docs/architecture.md`
- example export-format notes

## FS-023 - Add summary metrics for real-terrain scenario review

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- Raw per-cell depth output is necessary, but it is still cumbersome for quick scenario review.
- Users interested in real scenarios need a small set of summary numbers to compare runs before a full map viewer exists.

Proposed change:
- Compute and emit a minimal metrics summary for a run, such as total water volume proxy, maximum depth, wet-cell count, and deepest-cell location.
- Keep the metrics derivable from the current raster model without claiming hydrologic realism beyond the MVP.

Constraints:
- Do not add heavyweight reporting dependencies.
- Keep metrics semantics clearly documented as raster-model summaries.

Acceptance criteria:
- The example workflow emits a small deterministic summary alongside CSV output.
- Metrics are documented and easy to compare across runs.
- The implementation does not alter simulation behavior.

Required tests:
- Add coverage for metric computation on small deterministic grids.
- Update example smoke tests if they validate summary output.

Required documentation updates:
- `README.md`
- `examples/real_terrain/README.md`
- `docs/architecture.md` if a new output artifact is added

## FS-030 - Add an open-boundary option for real-terrain edge behavior

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- Real-terrain clips currently use closed boundaries only, which can trap water unrealistically at the edges of a clipped domain.
- That weakens real-world interpretability even if the ingestion and scenario workflow are otherwise clean.

Proposed change:
- Add a narrow open-boundary or edge-outflow mode that can be selected explicitly for real-terrain runs.
- Keep the first implementation simple and deterministic rather than introducing a drainage-network model.

Constraints:
- Preserve the current documented closed-boundary behavior as one supported mode.
- Make the new behavior explicit in docs and outputs so runs cannot be confused.
- Avoid mixing this ticket with broader urban-drainage work.

Acceptance criteria:
- A real-terrain run can choose between the current closed-boundary behavior and one documented edge-outflow mode.
- Tests pin the behavioral difference on small deterministic grids.
- Example output or metadata makes the chosen boundary mode visible.

Required tests:
- Add simulation coverage for the new boundary behavior.
- Update real-terrain example coverage if the user-facing workflow changes.

Required documentation updates:
- `README.md`
- `docs/architecture.md`
- `docs/simulation_model.md`
- `examples/real_terrain/README.md`

## FS-031 - Add a simple rainfall-loss or infiltration control

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The current model turns all rainfall into surface water, which limits real-world usefulness even for rough screening scenarios.
- Without at least one simple loss term, scenario comparisons can become operationally tidy but physically thin.

Proposed change:
- Add one narrow rainfall-loss or infiltration control, such as a uniform per-step loss rate or runoff coefficient, for the current raster model.
- Keep the first version intentionally simple and well documented.

Constraints:
- Do not present the feature as calibrated hydrology.
- Preserve deterministic behavior and clear mass-accounting semantics.
- Avoid coupling this ticket to land-use layers or drainage-network inputs.

Acceptance criteria:
- A run can apply a documented simple loss control that changes total retained surface water in a predictable way.
- Tests cover zero-loss, nonzero-loss, and invalid parameter handling.
- Docs state clearly what approximation is being made and what it does not model.

Required tests:
- Add deterministic simulation tests for the loss-control behavior.
- Update example or smoke coverage if the feature is exposed through the real-terrain workflow.

Required documentation updates:
- `README.md`
- `docs/architecture.md`
- `docs/simulation_model.md`
- `examples/real_terrain/README.md`

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
