# AGENTS.md

Guidance for future Codex work in this repository:

- Keep changes small, buildable, and easy to review.
- Explain technical tradeoffs, especially when changing simulation behavior or project structure.
- Always update documentation when architecture, assumptions, or simulation behavior change.
- Always run the build and tests after code changes when the environment allows it.
- Prefer C++20, CMake, clear tests, and minimal dependencies.
- Avoid adding heavy external libraries unless there is a strong, explicit reason.
- Do not introduce web, backend, or frontend stacks yet unless explicitly requested.
- Preserve the current MVP posture: simple raster/grid simulation first, richer hydrology later.

## Minimal workflow

Use a lightweight role split when it improves clarity, but do not force multiple agents for small tasks.

### Analyst

- Read the affected code, tests, and relevant docs before proposing a change.
- Identify the smallest correct implementation scope.
- Call out risks when simulation semantics, exported formats, or project structure may change.
- Check whether ticket or roadmap notes should be updated.

### Implementer

- Make the smallest buildable change that satisfies the task.
- Keep interfaces and dependencies simple unless expansion is explicitly required.
- Update docs when behavior, architecture, assumptions, or output formats change.

### Tester

- Run `cmake --build build` after code changes when the environment allows it.
- Run `ctest --test-dir build --output-on-failure` after code changes when the environment allows it.
- Run a relevant example or smoke test when user-facing behavior, CLI output, or exported files change.
- Call out missing coverage if behavior changed but tests did not.

## Definition of done

- The change is buildable.
- Tests pass when the environment allows them to run.
- Docs are updated when simulation behavior, architecture, assumptions, or outputs changed.
- Ticket tracking is updated when the task was driven by `docs/tickets.md`.

## When to split roles

- Use explicit analyst, implementer, and tester passes for simulation-behavior changes, export-format changes, or structural refactors.
- Use a single compact pass for small doc-only work, narrow test additions, or straightforward cleanup with no behavior change.
