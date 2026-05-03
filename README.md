# FloodSim

FloodSim is an urban flood simulation product focused on rainfall-driven flooding over real city terrain. The long-term goal is climate-risk simulation for cities, but the first milestone is intentionally small: a clear, buildable raster-grid prototype that can evolve into a more capable simulation stack.

## Product vision

FloodSim aims to support workflows such as:

- loading terrain and urban surface data
- simulating rainfall accumulation and overland flow
- exporting flood-depth outputs for maps and downstream analysis
- comparing infrastructure and climate scenarios over time

## MVP scope

This repository starts with a simplified first model:

- C++20 simulation core
- raster/grid terrain representation
- uniform rainfall over a grid
- simple downhill neighbor flow
- lightweight tests
- Python placeholders for future preprocessing and DEM ingestion

This is not a certified hydrology or hydrodynamics engine. The MVP is designed for clarity, testability, and iterative improvement.

## Repository layout

```text
flood-sim/
  core/               C++ simulation library and tests
  data/scripts/       preprocessing and ingestion helpers
  docs/               architecture, model notes, roadmap
  examples/           runnable toy examples
  AGENTS.md           instructions for future Codex work
```

## Build

Requirements:

- CMake 3.20+
- a C++20 compiler such as GCC 11+, Clang 14+, or MSVC with C++20 support

Build commands:

```bash
cmake -S . -B build
cmake --build build
```

## Run the example

```bash
./build/floodsim_simple_grid
```

The example runs a toy `10x10` terrain, applies rainfall for several steps, and prints water-depth totals plus the final grid.

It can also export a self-describing CSV file:

```bash
./build/floodsim_simple_grid final_grid.csv
python3 examples/simple_grid/inspect_export.py final_grid.csv
```

The companion consumer example reads the exported Phase 1 CSV contract and prints a compact summary of the grid metadata and simulated water results.

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Next steps

The intended evolution is:

1. keep the toy grid stable and well tested
2. add DEM import, likely through GDAL integration
3. produce exportable raster outputs for visualization
4. layer in richer rainfall and urban-surface behavior

More detail is in [docs/architecture.md](/home/sergio/dev/flood-sim/docs/architecture.md), [docs/simulation_model.md](/home/sergio/dev/flood-sim/docs/simulation_model.md), and [docs/roadmap.md](/home/sergio/dev/flood-sim/docs/roadmap.md).

## Phase 1 paper

The repository also includes a LaTeX paper for the implemented Phase 1 simulator in [docs/paper/README.md](/home/sergio/dev/flood-sim/docs/paper/README.md:1). It documents the toy-grid model, rainfall and boundary contracts, routing pseudocode, representative figures, and the main limitations of the current phase.
