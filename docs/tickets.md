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

## FS-009 - Write a Phase 1 LaTeX simulator paper

Status: Todo
Owner: Unassigned
Priority: P3

Problem:
- Phase 1 is gaining stable semantics, tests, and exported outputs, but there is no single technical paper that explains the simulator as a coherent artifact.
- A compact paper would make the MVP easier to review, present, and extend during later phases.

Proposed change:
- Add a LaTeX paper that describes the Phase 1 simulator, its assumptions, and its intended scope.
- Include theory and modeling assumptions, representative graphs or figures, and pseudocode for the step algorithm.

Constraints:
- Keep the paper aligned with the implemented Phase 1 behavior rather than aspirational future features.
- Avoid turning the document into a claim of scientific validation or a certified hydrology model.
- Prefer figures and graphs that can be reproduced from repository examples or tests.

Acceptance criteria:
- The repository contains a buildable LaTeX paper source for the Phase 1 simulator.
- The paper explains the toy-grid model, rainfall contract, routing semantics, and boundary behavior.
- The paper includes at least one algorithm-style pseudocode section and at least one representative graph or figure.
- The paper clearly states limitations and non-goals of the Phase 1 simulator.

Required tests:
- Build or smoke-test instructions for generating the paper output.
- Validation that any generated figures or referenced example outputs can be reproduced from repo artifacts.

Required documentation updates:
- `README.md`
- `docs/architecture.md`
- paper build/use notes in a new paper-specific README or docs section

## In Progress

No tickets in progress.

## Blocked

No blocked tickets.

## Done
