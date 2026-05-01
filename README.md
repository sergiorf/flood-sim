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

