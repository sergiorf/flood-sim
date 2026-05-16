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
- The next product milestone is a layered, city-specific screening demo rather than a broader architecture refactor.
- New tickets should bias toward city onboarding, layer alignment, viewer usability, and practical simulation inputs before cross-engine generalization.

## FS-046 - Define a city onboarding protocol for repeatable demo and screening areas

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- Real city onboarding is still ad hoc.
- FloodSim now has a terrain library and a first Brussels terrain candidate, but there is no formal protocol for deciding which layers are required, which sources are preferred, or what makes one city area demo-ready.
- Without a protocol, every city target will create avoidable inconsistency in provenance, licensing, CRS handling, and workflow expectations.

Proposed change:
- Define a city onboarding protocol for FloodSim.
- Specify:
- mandatory versus optional layers
- preferred source order and fallbacks
- provenance and license requirements
- CRS and extent alignment rules
- demo-readiness and review states

Constraints:
- Keep the first protocol focused on the current screening product posture.
- Do not turn this into a broad ingestion framework yet.
- Prefer a clear checklist and repeatable documentation contract over premature automation.

Acceptance criteria:
- The repository documents a repeatable onboarding checklist for one city area.
- Required and optional layer types are explicit.
- Review states such as candidate, reviewed, and active demo are tied to concrete evidence.
- The Brussels demo area can be evaluated against that protocol.

Required tests:
- No numerical test changes are required for the planning pass.
- If scripts or validators are added, they should be covered by at least one small workflow or unit test.

Required documentation updates:
- `docs/terrain_library.md`
- `docs/roadmap.md`
- `README.md`

## FS-047 - Define a unified city layer model above terrain-only inputs

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- FloodSim currently has terrain plus a narrow surface-class concept, but the product direction now clearly requires more city layers such as buildings, land cover, and drainage features.
- Without a unified layer model, those additions risk becoming a blob of special-case rasters and overlays.

Proposed change:
- Define a clean city data model that separates:
- terrain
- surface or land-cover layers
- structures such as buildings or barriers
- drainage or outlet features
- provenance and alignment metadata

Constraints:
- Keep the model narrow enough for the Brussels demo milestone.
- Avoid turning this into a generic GIS schema.
- Preserve the current terrain-first MVP while making the next layers fit coherently.

Acceptance criteria:
- The repository documents the target layer categories and their responsibilities.
- The model makes clear which layer types are raster-aligned and which may remain vector-based before rasterization.
- The model is compatible with both the simulation workflow and the native viewer direction.

Required tests:
- No solver behavior change is required for the design pass.
- If new public structs or parsing code are introduced, add focused tests around invariants and alignment metadata.

Required documentation updates:
- `docs/architecture.md`
- `docs/terrain_library.md`
- `docs/simulation_model.md`

## FS-045 - Build a lean native viewer MVP for large terrain and result rasters

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The current Python/Tk debug viewer is useful for tiny fixtures but does not scale to real terrain clips.
- Real-terrain testing now benefits from larger local DEMs such as the Brussels candidate clip, and the current per-cell canvas rendering path becomes unusable there.
- The product needs a fast local viewer that can become part of the suite, not only a developer debug utility.

Proposed change:
- Build a lean native local viewer optimized for FloodSim raster inspection.
- Use a lightweight rendering stack rather than a heavy desktop framework.
- First scope should focus on fast raster display for:
- terrain rasters
- FloodSim CSV result rasters
- snapshot series or multi-frame result inspection

Constraints:
- Keep the first version local-first and file-based.
- Do not build a full GIS or broad application shell yet.
- Prefer a lightweight native stack with strong raster rendering performance and low integration overhead.
- Keep the MVP focused on viewing and inspection before simulation orchestration.

Acceptance criteria:
- The viewer can open a real-terrain raster larger than the current tiny fixtures without locking up the UI.
- The viewer can render at least terrain elevation and FloodSim CSV result layers efficiently.
- The viewer supports basic pan, zoom, and cell inspection.
- The viewer can step through a snapshot series or equivalent multi-frame run output when provided.

Required tests:
- Add at least one automated smoke path that exercises viewer input parsing or render-preparation logic on a larger raster case.
- Document any manual performance acceptance checks required for the native UI path.

Required documentation updates:
- `docs/roadmap.md`
- `docs/architecture.md`
- `README.md`

## FS-048 - Add layer management to the native viewer

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The native viewer now has a product-facing home, but it will not scale if it only treats every input as one flat raster.
- The city product direction requires several aligned layers such as elevation, buildings, land cover, and simulation results.

Proposed change:
- Add a layer-management system to the native viewer.
- Support:
- loading multiple layers for one area
- layer visibility and ordering
- metadata display for the active layer
- switching between terrain, inputs, and result layers without backend-specific logic leaking through the app

Constraints:
- Keep the first version local-first and file-based.
- Do not build a full GIS legend or styling engine yet.
- Preserve the backend abstraction so future graphics framework changes stay localized.

Acceptance criteria:
- The native viewer can represent more than one layer for a city area.
- The active layer can be switched without rebuilding the whole application structure.
- Layer metadata is inspectable in a clear way.

Required tests:
- Add focused tests for layer manifest parsing or viewer-side layer selection logic when those pieces are introduced.
- Document any manual UI checks required for the native viewer MVP.

Required documentation updates:
- `docs/architecture.md`
- `docs/roadmap.md`
- `README.md`

## FS-049 - Improve simulation configuration for layered city screening inputs

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The current scenario configuration is still centered on storm parameters and a narrow runoff overlay.
- As soon as city onboarding adds buildings, land-cover, or drainage-related layers, the simulation configuration needs a clearer way to reference and explain them.

Proposed change:
- Extend the simulation-facing workflow configuration so layered city inputs can be selected, validated, and reported explicitly.
- Keep the first pass screening-focused and explainable.

Constraints:
- Do not jump to a fully general scenario-management system.
- Keep the current toy model and its semantics understandable.
- Prefer practical configuration seams over a premature cross-engine abstraction.

Acceptance criteria:
- A scenario can identify which layered city inputs were active for a run.
- Reports and exports preserve enough metadata to explain those inputs safely.
- The configuration remains compact and practical for the Brussels demo milestone.

Required tests:
- Add workflow tests for any new scenario or layer-reference contract.
- Preserve existing deterministic behavior when no new layers are active.

Required documentation updates:
- `docs/simulation_model.md`
- `examples/real_terrain/README.md`
- `README.md`

## FS-050 - Assemble a canonical Brussels demo package

Status: Todo
Owner: Unassigned
Priority: P1

Problem:
- The repository now has a Brussels terrain candidate, but not yet a complete canonical demo package.
- A convincing screening demo needs more than elevation alone: it needs a chosen area, documented source set, scenario set, and a clear visual/testing path.

Proposed change:
- Assemble one canonical Brussels demo package with:
- one exact area definition
- one trusted terrain source
- at least one additional city layer candidate such as buildings or land cover
- a documented scenario set
- a repeatable inspection workflow through the native viewer direction

Constraints:
- Keep the scope narrow and demo-focused.
- Do not wait for a fully general city-ingestion system before making one concrete package work.
- Preserve explicit screening-only positioning.

Acceptance criteria:
- One Brussels area is documented as the canonical demo target.
- The demo package identifies its layer sources and review status.
- The repository can walk a user from staged data to simulation output to viewer inspection on that area.

Required tests:
- Add or update at least one smoke path that uses the canonical Brussels package in a repeatable way.
- Document any manual demo validation steps that remain outside automated coverage.

Required documentation updates:
- `terrain_library/areas/brussels_demo_center/README.md`
- `docs/roadmap.md`
- `README.md`

## FS-043 - Refactor architecture to support screening and engineering model engines

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The current architecture is centered on one in-repo toy raster model.
- That is appropriate for screening and product iteration, but it is not a strong long-term product posture if the product may later support real engineering workflows.
- Without an explicit engine boundary, future integration of an accepted external model risks leaking product concerns into solver-specific code and weakening the clarity of result claims.

Proposed change:
- Introduce a narrow engine-agnostic execution contract so the product workflow can run either:
- the current in-repo toy model as a screening engine
- a future accepted open-source external model as an engineering engine
- Move shared ingestion, scenario normalization, output identity, and comparison logic above that engine boundary.
- Ensure every run artifact identifies which engine produced it and what level of confidence or intended use applies.

Constraints:
- Preserve the current toy model and its deterministic regression fixtures as the local screening path.
- Do not claim that integrating an accepted model makes the whole product certified.
- Keep the first refactor focused on architecture seams, contracts, and workflow boundaries rather than full external-model feature parity.
- Prefer open-source and free-to-use engineering-model candidates where they fit the intended use case.

Acceptance criteria:
- The repository documents a clear engine boundary and the responsibilities above and below it.
- A shared run contract exists that can represent at least the current screening engine and one future engineering-engine adapter path.
- Exports, reports, or run metadata distinguish screening-mode outputs from engineering-mode outputs.
- The current toy model remains usable without any external-engine dependency.

Required tests:
- Regression coverage for the screening-engine path must remain green after the boundary extraction.
- New tests should constrain engine identity metadata and any engine-selection workflow behavior introduced by the refactor.

Required documentation updates:
- `docs/architecture.md`
- `docs/simulation_model.md`
- `docs/roadmap.md`
- `README.md`

## FS-040 - Add explicit outlet or sink features beyond raster-edge open boundaries

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The current `Open` boundary only lets water leave across missing orthogonal neighbors at the raster edge.
- Real screening workflows often need at least one explicit outlet, culvert-like sink, or designated discharge point inside the modeled domain.

Proposed change:
- Add one narrow outlet/sink contract so selected cells can discharge water according to a simple explicit rule instead of relying only on raster-edge escape.

Constraints:
- Keep the outlet behavior easy to reason about and test.
- Do not turn this into a full pipe-network or hydraulic-structure subsystem yet.
- Preserve the existing boundary-mode semantics for runs that do not use outlets.

Acceptance criteria:
- A scenario can define at least one explicit outlet or sink location.
- Outlet-enabled runs differ predictably from pure edge-boundary runs on committed fixtures.
- Reports and exports identify when outlet behavior was active.

Required tests:
- Regression coverage for at least one fixture where an explicit outlet changes retained water and deepest-cell behavior.

Required documentation updates:
- `docs/simulation_model.md`
- `examples/real_terrain/README.md`

## FS-051 - Review and tighten product and architecture documentation after the layered-city shift

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The repository documentation has grown alongside the MVP, but the new direction toward city onboarding, layered data, and a native viewer will make terminology drift likely.
- Without a review pass, different docs may keep describing different product shapes.

Proposed change:
- Review and tighten the main product, simulation, terrain-library, and architecture docs after the next layered-city milestones land.

Constraints:
- Prefer tightening terminology and responsibilities over rewriting for style.
- Keep docs aligned with implemented reality.

Acceptance criteria:
- The main docs use consistent terminology for terrain, layers, viewer, and screening posture.
- Outdated references to older workflow assumptions are removed or clarified.

Required tests:
- No code tests required unless doc-driven interface changes expose missing coverage.

Required documentation updates:
- `README.md`
- `docs/architecture.md`
- `docs/simulation_model.md`
- `docs/terrain_library.md`

## FS-042 - Harden core API documentation and split overloaded example support code

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The repository now has meaningful simulation and workflow contracts, but some public types and example-support responsibilities are still under-documented or concentrated in a few files.
- This raises the cost of adding the next MVP features safely.

Proposed change:
- Continue documenting core public types and narrow contracts, and split any remaining overloaded example-support code where parsing, execution, and reporting are still too tightly coupled.

Constraints:
- Prefer small structural cleanup tied to active responsibilities.
- Do not rewrite stable code just for style.
- Keep public interfaces compact.

Acceptance criteria:
- Public headers and workflow entry points document their responsibilities and invariants clearly enough for follow-on MVP work.
- Any remaining overloaded example-support module is reduced or clarified without changing behavior.
- Tests remain green with no behavioral drift.

Required tests:
- Existing build and regression suite should pass unchanged unless a cleanup reveals missing coverage.

Required documentation updates:
- `docs/architecture.md`
- `docs/simulation_model.md`
- any touched public header comments

## FS-044 - Define a reusable simulation-platform boundary above flood-specific engines

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- The repository may eventually support other simulator products beyond flood modeling.
- Today, the architecture is still centered on flood-specific concepts such as terrain rasters, rainfall, and routing.
- Without a clean platform boundary, reusing the workflow stack for other domains such as RF or macroeconomics would require copy-paste rather than composition.

Proposed change:
- Define a narrow simulation-platform layer above domain-specific engines.
- Identify the reusable contracts for run requests, scenario identity, execution lifecycle, artifact metadata, result summaries, and comparison workflows.
- Keep flood-specific state and solver semantics below that boundary so the current repository does not over-generalize its numerical core.

Constraints:
- Do not force the current flood solver to pretend it is domain-agnostic.
- Prefer interfaces proven by near-term needs such as multiple flood-engine support before extracting broader abstractions.
- Keep the first pass architectural and contractual; do not expand scope into a full plugin marketplace or cross-domain product framework.

Acceptance criteria:
- The architecture documents which layers are reusable platform concerns and which layers remain flood-specific.
- A first-pass vocabulary exists for platform-level concepts such as run request, engine identity, artifact metadata, and result summary.
- The proposed boundary remains compatible with the screening-versus-engineering flood-engine split.

Required tests:
- No new numerical behavior is required for the planning pass.
- If code extraction begins, tests must preserve current flood-engine behavior unchanged while covering any new platform metadata or orchestration contracts.

Required documentation updates:
- `docs/architecture.md`
- `docs/roadmap.md`
- `README.md`

## FS-041 - Add planner-facing scenario comparison artifacts

Status: Todo
Owner: Unassigned
Priority: P2

Problem:
- The repository already emits deterministic CSV outputs, but the comparison experience is still developer-oriented.
- Existing market tools emphasize reviewable scenario summaries, time-series outputs, and result comparison artifacts, not only raw grid exports.

Proposed change:
- Add one narrow comparison bundle for real-terrain runs, such as richer summary tables plus deterministic snapshot or peak-depth comparison artifacts that can be inspected without custom scripting.

Constraints:
- Stay local and file-based.
- Do not add a web app or heavyweight reporting stack.
- Prefer artifacts that remain stable under regression testing.

Acceptance criteria:
- A batch or reviewed-scenario run produces at least one additional comparison artifact beyond the current per-scenario grid CSVs.
- The artifact makes uniform and profile-driven scenarios easier to compare safely.
- Tests constrain the generated artifact format.

Required tests:
- Workflow coverage for the comparison artifact in at least one batch scenario run.

Required documentation updates:
- `examples/real_terrain/README.md`
- `README.md`

## FS-052 - Remove or consolidate obsolete viewer and workflow code after the native path stabilizes

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- The repository now carries both temporary and future-facing viewer/workflow paths.
- Once the native viewer and layered city workflows stabilize, some temporary code may become confusing or redundant.

Proposed change:
- Remove or consolidate obsolete code paths after replacements are proven.
- Focus first on duplicated viewer or workflow logic that no longer pulls its weight.

Constraints:
- Do not remove useful debug utilities prematurely.
- Prefer evidence-based cleanup after replacement paths are stable and documented.

Acceptance criteria:
- Obsolete or redundant code is removed only when a stable replacement exists.
- Remaining viewer and workflow tools have clearly distinct roles.

Required tests:
- Existing tests should remain green, with updates where cleanup changes paths or invocation targets.

Required documentation updates:
- `README.md`
- any touched viewer or workflow docs

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
