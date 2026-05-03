# Phase 1 Paper

This directory contains the LaTeX source for the FloodSim Phase 1 simulator paper.

## Build

From the repository root:

```bash
cd docs/paper
latexmk -pdf phase1_simulator.tex
```

The expected output is:

```text
docs/paper/phase1_simulator.pdf
```

To clean generated paper artifacts:

```bash
cd docs/paper
latexmk -c phase1_simulator.tex
```

## Purpose

The paper is meant to document the implemented Phase 1 simulator as a technical artifact. It is not a claim of hydrologic validation or regulatory suitability.

The paper covers:

- modeling assumptions
- rainfall contract
- routing and boundary semantics
- pseudocode for the step algorithm
- representative figures
- limitations and non-goals
