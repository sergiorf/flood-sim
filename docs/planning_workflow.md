# Planning Workflow

This repository should stay lightweight in process while the simulation model is still evolving.

## Purpose of each planning layer

Use the planning artifacts for different levels of detail:

- `docs/roadmap.md`: phase-level direction for the project.
- iteration plan: the next 3 to 6 concrete tasks inside the current phase.
- tickets: bounded pieces of work that need discussion, reviewable scope, or follow-up.

## Suggested workflow

1. Choose the active roadmap phase.
2. Write a short iteration plan for that phase.
3. Break larger tasks into tickets with explicit acceptance criteria.
4. Keep small obvious work out of tickets when possible.
5. Update docs and tests whenever simulation behavior or architecture changes.
6. Revisit the iteration plan at the end of each small milestone.

## When to create a ticket

Create a ticket when the work:

- changes simulation behavior
- changes project structure or public interfaces
- introduces a new file format or import/export path
- requires a design decision or tradeoff to be recorded
- is likely to span more than one focused work session

Do not create a ticket for:

- small documentation edits
- narrow test additions
- straightforward cleanup or refactors with no behavior change
- tiny example improvements

## Ticket template

Each ticket should be small enough to review and finish independently.

Recommended fields:

- Title
- Problem
- Proposed change
- Constraints
- Acceptance criteria
- Required tests
- Required documentation updates

Example:

`Add configurable boundary conditions to the grid simulation`

- Problem: edge handling is currently implicit and not documented as a modeling choice.
- Proposed change: add a configurable boundary mode and apply it consistently in the step logic.
- Constraints: keep the raster MVP simple and avoid heavy dependencies.
- Acceptance criteria: closed and open boundary modes exist, tests cover both, docs explain behavior.
- Required tests: conservation and edge outflow behavior.
- Required documentation updates: `docs/simulation_model.md` and example notes.

## Iteration plan template

The iteration plan can live in a short document or in issues/milestones if you later adopt them.

Suggested format:

```text
Current phase: Phase 1. Toy grid simulation

Current milestone goal:
- Lock down core simulation semantics and observability.

Next tasks:
- Add tests for multi-neighbor flow and flat terrain.
- Document the meaning of surface height and delta accumulation.
- Add a simple CSV export for example outputs.
- Decide and document boundary behavior.

Deferred:
- DEM import
- graphical viewer
```

## Graphical app guidance

A graphical app is useful for this project, but it should follow stable outputs rather than lead core modeling work.

Recommended sequence:

1. keep the CLI example working
2. add exportable raster or CSV outputs
3. build a lightweight viewer around those outputs
4. only then consider a richer interactive application
