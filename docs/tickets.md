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

## FS-043 - Refactor architecture to support screening and engineering model engines

Status: Todo
Owner: Unassigned
Priority: P1

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

## FS-044 - Define a reusable simulation-platform boundary above flood-specific engines

Status: Todo
Owner: Unassigned
Priority: P2

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

## FS-040 - Add explicit outlet or sink features beyond raster-edge open boundaries

Status: Todo
Owner: Unassigned
Priority: P1

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

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.
