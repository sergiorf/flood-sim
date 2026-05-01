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

